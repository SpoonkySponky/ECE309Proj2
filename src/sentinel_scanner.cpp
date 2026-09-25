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
        int foundPos = maybeText.find(sentinel_); //find the end text
        if(foundPos != std::string::npos){ //if the end text is actually found...
            string safeText = maybeText.substr(0, foundPos);//keeps the text from 0 to end point
            pending_ = ""; //clears out pending

            Out result;                       
            result.safe_text = safeText;      
            result.sentinel_found = true;     
            return result; 
        }else{ //if the end text isn't found, it may just be a small chunk of it
               //hold onto some characters in case the end string continues in the next chunk
            int holdOntoText = sentinel_.size() - 1;
            if (holdOntoText > maybeText.size()) { //checks to make sure holdOntoText isn't larger
                holdOntoText = maybeText.size();   //than maybeText, else set them equal
            }

            int newSafeText = maybeText.size() - holdOntoText; //this subtracts the text held onto
                                                            //because it contains some of the end string
                                                            //so the new stuff is confirmed safe

            string safeText = maybeText.substr(0, newSafeText); //safeText is now the new bit of safe text
            pending_ = maybeText.substr(newSafeText); //sets pending_
            Out result;                       
            result.safe_text = safeText;      
            result.sentinel_found = false;     
            return result; 
        }
    }

    // Call once, after the stream ends, to release any text still
    // being held back.
    Out flush(){
        ~pending_;
    }

private:
    std::string sentinel_;
    std::string pending_;   // holds back at most sentinel_.size() - 1
                             // trailing characters that could still
                             // become the start of the sentinel
};