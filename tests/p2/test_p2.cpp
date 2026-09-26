#undef NDEBUG //makes sure assert() always runs, even in release builds!
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "core/conversation.h"
#include "core/sentinel_scanner.h"
#include "model/scripted_client.h"
#include "model/replay_client.h"
#include "harness/harness.h"
using namespace std;

    const std::string SENTINEL = "<|end_conversation|>";

    bool contains(const std::string& text, const std::string& part){
        std::size_t position = text.find(part);
        if(position == std::string::npos){ 
            return false;
        }
        return true;
    }

    void write_file(const std::string& path, const std::string& text){
        std::ofstream file(path);
        file << text;
        file.close();
    }

    std::string scan(const std::string& text, std::size_t chunk, bool& found){
        SentinelScanner scanner(SENTINEL);
        std::string output = "";
        found = false;

        for(std::size_t i = 0; i < text.size(); i += chunk){
            std::string piece = text.substr(i, chunk); 

            SentinelScanner::Out out = scanner.feed(piece);
            output += out.safe_text; 
            if(out.sentinel_found){ 
                found = true;
                return output;
            }
        }
        SentinelScanner::Out leftover = scanner.flush();
        output += leftover.safe_text;
        return output;
    }

    class FakeInput : public InputSource{
    public:
        std::string lines[10]; 
        int count = 0; 
        int next = 0;
        bool eof = false;  

        void add_line(const std::string& line){
            if(this->count >= 10){
                throw std::out_of_range("FakeInput is full :(");
            }
            this->lines[this->count] = line;
            this->count++;
        }

        std::string read_line() override{
            if(next < count){ 
                std::string line = lines[next];
                next++;
                return line;
            }
            eof = true;
            return "";
        }

        bool is_eof() const override{
            return eof;
        }
    };

    class FakeOutput : public OutputSink{
    public:
        std::string text = ""; 

        void write(std::string_view t) override{
            text.append(t); 
        }
    };

    StopReason run_script(const std::string& script, int max_turns, std::string& printed){
        std::string path = "test.script";
        write_file(path, script);
        HarnessConfig cfg;
        cfg.max_turns = max_turns;

        std::unique_ptr<ScriptedModelClient> client = std::make_unique<ScriptedModelClient>(path);
        Harness harness(std::move(client), cfg);
        FakeInput in;
        for(int i = 0; i < 5; i++){ 
            in.add_line("hello");
        }

        FakeOutput out;

        StopReason reason = harness.run(in, out);
        printed = out.text;

        std::remove(path.c_str()); 
        return reason;
    }

    void test1_empty(){
        Conversation conv;

        assert(conv.size() == 0);
        assert(conv.begin() == conv.end());

        int count = 0;
        for(const Message& m : conv){
            (void)m; 
            count++;
        }
        assert(count == 0); 
    }

    void test2_system_first(){
        Conversation conv;
        conv.append(Message(Role::System, "Be concise."));
        for(int i = 0; i < 50; i++){ 
            conv.append(Message(Role::User, "hi"));
        }

        Conversation copy = conv;
        Conversation moved = std::move(copy);

        assert(conv.at(0).role() == Role::System);  
        assert(moved.at(0).role() == Role::System); 
        assert(moved.at(0).content() == "Be concise.");
    }

    void test3_copy(){
        Conversation a;
        a.append(Message(Role::User, "hello"));

        Conversation b(a);

        assert(b.begin() != a.begin());
        assert(b.at(0).content() == "hello");
        b.append(Message(Role::User, "more"));
        assert(a.size() == 1); 

        Conversation c;
        c = a;
        assert(c.begin() != a.begin());
        assert(c.at(0).content() == "hello");

        Conversation& same = c;
        c = same;
        assert(c.size() == 1);
    }

    void test4_move(){
        Conversation a;
        a.append(Message(Role::User, "hello"));
        const Message* buffer = a.begin();

        Conversation b(std::move(a)); 
        assert(b.begin() == buffer);  
        assert(a.size() == 0);
        assert(a.begin() == nullptr);

        Conversation c;
        c = std::move(b); 
        assert(c.begin() == buffer);
        assert(b.size() == 0);
        assert(b.begin() == nullptr);
    }

    void test5_growth(){
        Conversation conv;
        std::size_t expected_capacity = 0;

        for(std::size_t i = 0; i < 100; i++){ 
            bool should_grow = false;
            if(conv.size() == expected_capacity){
                should_grow = true;
                if(expected_capacity == 0){ 
                    expected_capacity = 1;  
                }else{
                    expected_capacity *= 2; 
                }
            }

            const Message* before = conv.begin();
            conv.append(Message(Role::User, std::to_string(i)));
            const Message* after = conv.begin();

            bool grew = false;
            if(before != after){
                grew = true;
            }
            assert(grew == should_grow);

            assert(conv.size() == i + 1);
            for(std::size_t j = 0; j <= i; j++){
                assert(conv.at(j).content() == std::to_string(j));
            }
        }
    }

    void test6_clean_text(){
        const std::string text = "Hello! No sentinel here.";
        bool found = false;
        std::string result = "";

        result = scan(text, text.size(), found);
        assert(result == text);
        assert(found == false);

        result = scan(text, 1, found);
        assert(result == text);
        assert(found == false);
    }

    void test7_split_everywhere(){
        const std::string text = "Goodbye." + SENTINEL;

        for(std::size_t split = 0; split <= text.size(); split++){ 
            std::string first_part = text.substr(0, split);
            std::string second_part = text.substr(split);

            SentinelScanner scanner(SENTINEL);
            SentinelScanner::Out out1 = scanner.feed(first_part);
            SentinelScanner::Out out2 = scanner.feed(second_part);

            if(split < text.size()){
                assert(out1.sentinel_found == false); 
            }

            bool found = false;
            if(out1.sentinel_found || out2.sentinel_found){
                found = true;
            }
            assert(found == true);

            std::string all_output = out1.safe_text + out2.safe_text;
            assert(all_output == "Goodbye.");
        }
    }

    void test8_false_alarms(){
        const std::string fakes[] = {"<|end_world|>", "<|end_conversation", "<|end_conversation|"};
        bool found = false;
        std::string result = "";

        for(const std::string& text : fakes){ 
            result = scan(text, 1, found);
            assert(result == text);
            assert(found == false);
        }

        result = scan("<|end_" + SENTINEL, 1, found);
        assert(result == "<|end_");
        assert(found == true);
    }

    void test9_bounded_memory(){
        std::string stream = "";
        while(stream.size() < 4 * 1024 * 1024){ 
            stream += "<|end_"; 
        }

        SentinelScanner scanner(SENTINEL);
        std::size_t fed = 0;
        std::size_t returned = 0;
        std::size_t max_held = SENTINEL.size() - 1; 

        for(char c : stream){ 
            std::string one_char(1, c);

            SentinelScanner::Out out = scanner.feed(one_char);
            fed++;
            returned += out.safe_text.size();

            assert(out.sentinel_found == false);

            std::size_t held = fed - returned;
            assert(held <= max_held);
        }

        //nothing lost: flush gives back the rest
        SentinelScanner::Out leftover = scanner.flush();
        returned += leftover.safe_text.size();
        assert(returned == fed);
    }

    // The harness stops after max_turns and never prints later replies.
    void test10_turn_limit(){
        std::string script = "";
        script += "role: assistant\nreply one\n---\n";
        script += "role: assistant\nreply two\n---\n";
        script += "role: assistant\nreply three\n";

        std::string printed = "";
        StopReason reason = run_script(script, 2, printed);

        assert(reason.kind == StopReason::Kind::TurnLimit);
        assert(contains(printed, "reply one") == true);
        assert(contains(printed, "reply three") == false);
    }

    // The harness stops right away when the sentinel shows up, and hides it.
    void test11_sentinel_halt(){
        std::string script = "";
        script += "role: assistant\nHi there.\n---\n";
        script += "chunk: 3\n"; //stream this reply 3 characters at a time
        script += "role: assistant\nGoodbye.<|end_conversation|>\n---\n";
        script += "role: assistant\nnever printed\n";

        std::string printed = "";
        StopReason reason = run_script(script, 20, printed);

        assert(reason.kind == StopReason::Kind::Sentinel);
        assert(contains(printed, "Goodbye.") == true);
        assert(contains(printed, "<|end") == false);         //sentinel hidden
        assert(contains(printed, "never printed") == false); //stopped right away
    }

    void test12_round_trip(){
        std::string reply1 = "Hi! How can I help?";
        std::string reply2 = "4.";

        std::string transcript = "";
        transcript += "role: user\nhello\n---\n";
        transcript += "role: assistant\n" + reply1 + "\n---\n";
        transcript += "role: user\nwhat is 2+2\n---\n";
        transcript += "role: assistant\n" + reply2 + "\n";

        std::string path = "test_transcript.txt";
        write_file(path, transcript);

        ReplayModelClient replay(path);
        ModelClient& model = replay; 

        Conversation conv;

        conv.append(Message(Role::User, "hello"));
        Message answer1 = model.generate(conv);
        assert(answer1.content() == reply1);

        conv.append(answer1);
        conv.append(Message(Role::User, "what is 2+2"));
        Message answer2 = model.generate(conv);
        assert(answer2.content() == reply2);

        std::remove(path.c_str()); 
    }

    int main(){
        test1_empty();
        std::cout << "PASS test 1!\n";

        test2_system_first();
        std::cout << "PASS test 2!\n";

        test3_copy();
        std::cout << "PASS test 3!\n";

        test4_move();
        std::cout << "PASS test 4!\n";

        test5_growth();
        std::cout << "PASS test 5!\n";

        test6_clean_text();
        std::cout << "PASS test 6!\n";

        test7_split_everywhere();
        std::cout << "PASS test 7!\n";

        test8_false_alarms();
        std::cout << "PASS test 8!\n";

        test9_bounded_memory();
        std::cout << "PASS test 9!\n";

        test10_turn_limit();
        std::cout << "PASS test 10!\n";

        test11_sentinel_halt();
        std::cout << "PASS test 11!\n";

        test12_round_trip();
        std::cout << "PASS test 12!\n";

        std::cout << "All tests passed! :D\n";
        return 0;
    }