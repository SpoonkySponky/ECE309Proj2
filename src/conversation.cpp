#include <stdexcept>
#include "core\conversation.h"
#include "core/message.h"
using namespace std;

class Conversation {
public:
    // Empty conversation: size() == 0, no allocation yet.
    Conversation(){
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    };

    // Releases all owned Message storage. No effect if already empty
    // (e.g. moved-from).
    ~Conversation(){    //destructor
        delete[] data_;
    };

    // Deep copy: allocates its own buffer and copies every Message.
    // this->begin() must differ from other.begin() afterward.
    Conversation(const Conversation& other){    //copy constructor
        this->size_ = other.size_;  
        this->capacity_ = other.capacity_;
        if(other.capacity_ == 0){ //if there is nothing in the array, no need to copy!
            return;
        }
        this->data_ = new Message[other.capacity_]; //make  new array for data with the size of other.capacity_
        for(int i = 0; i < size_; i++){ //while i is less than the size of the amount of 
                                        // spaces with value in data, loop!
            this->data_[i] = other.data_[i];  //copies data into the new data array
        }
        
    }

    Conversation& operator=(const Conversation& other){ //assignment operator
        if(this != &other){
            delete[] data_; //frees the space in memory from data_
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = nullptr;

            if(other.capacity_ != 0){ //if there is nothing in the array, no need to assign!
            //copy like the constructor does now 
            this->data_ = new Message[other.capacity_]; //make  new array for data with the size of other.capacity_
            for(int i = 0; i < size_; i++){ //while i is less than the size of the amount of 
                                            // spaces with value in data, loop!
                this->data_[i] = other.data_[i];  //copies data into the new data array
            }
            }
        }
        return *this;
    }

    // Steals other's buffer — no per-element copying. Afterward, other
    // must be left valid and empty (safe to destroy or reassign).
    Conversation(Conversation&& other) noexcept{ //move constructor time!
        data_ = other.data_;    //steals the address of other.data_
        size_ = other.size_;     //steals both size and capacity of other
        capacity_ = other.capacity_;

        //sets all of other's info to 0/null, making it go away!
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }


    Conversation& operator=(Conversation&& other) noexcept{
        if(this != &other){
            delete[] data_; //frees the space in memory from data_
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;

            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // Appends m, growing the backing array if needed. Amortized O(1) —
    // document and justify your growth strategy in the design log
    // (see Appendix C if you want a refresher first).
    void append(Message m){
        std::size_t newCapacity = capacity_; //makes new capacity so the code is easier to read
        if(size_ == newCapacity){ //if size is equal to capacity, that means capacity needs to grow
            if(newCapacity == 0){ //if capacity is empty,
                newCapacity = 1; //then it shouldn't be! Change it to 1
            }else{
                newCapacity *= 2; //else, just double capacity, keeps the O(1) thing
            }
        
            Message* newData = new Message[newCapacity]; //newData is receiving the address of the 
                                                        //new rows being allocated
            for(int i = 0; i < size_; i++){ //loops for the size of the data in messages array
                newData[i] = data_[i];  //moves info from data_ into newData
            }
            delete[] data_; //Finally! I can delete data! Yay!!
            data_ = newData;   //now data_ can be newData
            capacity_ = newCapacity; //same for the capacity :D
        }
        data_[size_] = m; //new m goes into empty slot
        size_++; //make sure this addition is accounted for in size_!
    }

    // Number of messages currently stored.
    std::size_t size() const noexcept{
        return size_;
    }

    // Bounds-checked access. Decide what happens on i >= size() (throw,
    // assert, whatever you pick) and test that behavior explicitly.
    const Message& at(std::size_t i) const{
        if (i >= size_) {
        throw std::out_of_range("This is out of range :(");
    }
    return data_[i];
    }

    // Range-for iteration, oldest message first. begin() == end() when
    // size() == 0.
    const Message* begin() const noexcept{
        return data_;
    }
    const Message* end()   const noexcept{
        return data_ + size_;
    }

private:
    Message*    data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};