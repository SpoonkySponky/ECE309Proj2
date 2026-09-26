// tests/p2/test_p2.cpp
//
// YOUR test suite goes here. At least 12 assert-based test cases — see
// spec §5 for the required categories and the sample test for the
// expected level of rigor.
//
// This file is a stub so the project builds out of the box; replace the
// body of main() with your own tests.

#include "core/conversation.h"
#include "core/message.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include "model/scripted_client.h"

#include <cassert>
#include "core/conversation.h"
#include "core/message.h"
#include "core/sentinel_scanner.h"

#include <memory>
#include <sstream>

#include "harness/harness.h"
#include "model/scripted_client.h"

#include <fstream>

#include "model/replay_client.h"

class TestInput : public InputSource {
public:
    explicit TestInput(const std::string& text)
        : stream_(text) {}
    std::string read_line() override {
        std::string line;

        if (!std::getline(stream_, line)) {
            eof_ = true;
        }
        return line;
    }
    bool is_eof() const override {
        return eof_;
    }

private:
    std::istringstream stream_;
    bool eof_ = false;
}
class TestOutput : public OutputSink {
public:
    void write(std::string_view text) override {
        text_.append(text);
    }
    const std::string& text() const {
        return text_;
    }
private:
    std::string text_;
}
struct P2TestAccess {
    static std::size_t capacity(const Conversation& conversation) {
        return conversation.capacity_;
    }
    static std::size_t pending_size(const SentinelScanner& scanner) {
        return scanner.pending_.size();
    }
}

//Handle empty conversations without out-of-bounds access.
void TestEmptyConversationBounds(void){ //Test 1 
    Conversation conversation;

    assert(conversation.size() == 0);
    assert(conversation.begin() == conversation.end());
    
}

//Ensure system messages remain pinned at the front.
void TestSystemMessageOrdering(void){ //Test 2
    Conversation conversation;

    conversation.append(Message(Role::System, "System"));
    conversation.append(Message(Role::User, "Hello!"));
    conversation.append(Message(Role::Assistant, "Hi!"));
    conversation.append(Message(Role::User, "Bye!"));

    assert(conversation.size() == 4);
    assert(conversation.at(0).role() == Role::System);
    ssert(conversation.at(0).content() == Role::"System!");
    assert(conversation.at(1).role() == Role::User);
    assert(conversation.at(1).content() == Role::"Hello!");
    assert(conversation.at(2).role() == Role::Assistant);
    assert(conversation.at(2).role() == Role::"Hi!");
    assert(conversation.at(2).role() == Role::User);
    assert(conversation.at(2).role() == Role::"Bye!");
}


void ScannerCatchesSentinelAtEveryBoundary(void){ //Test 7 taken from example!
    const std::string sentinel = "<|end_conversation|>";
    const std::string text = "Goodbye." + sentinel;
    for (std::size_t split = 0; split <= text.size(); ++split) {
        SentinelScanner scanner(sentinel);
        auto out1 = scanner.feed(text.substr(0, split));
        auto out2 = scanner.feed(text.substr(split));
        assert((out1.sentinel_found || out2.sentinel_found) &&
               "sentinel must be caught regardless of split point");
        assert(out1.safe_text + out2.safe_text == "Goodbye.");
    }
}





