#include <string>
using namespace std;
class SentinelScanner {
public:
    explicit SentinelScanner(std::string sentinel){
        sentinel_ = move(sentinel);
    }

    struct Out { std::string safe_text; bool sentinel_found; };

    // Feed the next chunk. Returns text guaranteed NOT to be part of
    // the sentinel (safe to print immediately) and whether the
    // sentinel has now been fully seen.
    Out feed(std::string_view chunk){
        string maybeText = pending_; //maybeText holds pending_ which is the text that has
                                     //some of the end string we're looking for
        maybeText.append(chunk); //appends the rest of the message chunk

        if(contains.maybeText(sentinel_)){ //if the evil end text is actually found...
            endLocation = maybeText.find(sentinel_);//where is it?
            string safeText = maybeText.subtr(0, endLocation);//keeps the text from 0 to evil point
            return safeText;
        }
    }

    // Call once, after the stream ends, to release any text still
    // being held back.
    Out flush();

private:
    std::string sentinel_;
    std::string pending_;   // holds back at most sentinel_.size() - 1
                             // trailing characters that could still
                             // become the start of the sentinel
};