# Design Log — Project 2

(500–800 words total. See spec §5 for what each section must cover.)

## Growth factor and amortized cost
I chose my growth factor to be 2, leading to an amortized growth factor of O(1). In my code, I wrote that if capacity_ equals size_, then capacity needs to be updated. I chose to multiply my capacity_ by 2. This means that capacity is only being updated at sizes 1, 2, 4, 8, and so on all the way until 2n. Therefore, the amortized growth factor is O(1) as you look at the exponent of the n to find big O notation, and in this case n is to the first power.


## Rule of Five evidence
With the rule of five, the destructor makes sure to free all memory from Message. Then moving on to the copy constructor, I wrote an if/else statement to make sure the array wasn't empty before it begins to copy. Then, I wrote a for loop to make sure the data being copied does not overshoot the amount of data that is provided. The same applies to the assignment operator. In the move constructor, I made sure to free the space in memory from data_ as well as take the sizes from size and capacity and the address from data_. I also made sure not to copy the data if the array was empty, as that is unneccessary. This ensured that my code was free from any data leaks that may occur.


## Sentinel scanner: bounded pending_ proof
If the end string is found, that is running through an if/else statement. If the end string is found, you save the safeText, the text that you know does not contain the end string, then you clear out pending_ because it is no longer being used. If the end string is not found, then regarding pending_, sentinel_.size() - 1 is being used to store the amount of text being held onto. This text is then compared with the size of the text which we already said contained some of the end string, named maybeText. If the size we are holding is bigger than maybeText, we set the size of the text being held and make it equal to the size of the previous text. This makes sure we do not overshoot the amount of text needing to be saved into pending_. 

## What I would change differently

Personally, I would try to change the way I wrote the functions for conversation.cpp. I heavily referenced the slides while writing them, which I was able to understand the flow of the code from the lecture slides; however, I would want to rewrite them in a flow that is more intuitive to me. I would also make sure that I write better and more descriptive comments, as that is something I often struggle with when writing code. For this project, I made sure to write comments for each line that vaguely describe what the code is doing so that I may use my code for review and to revisit for future projects if it is applicable; however, my comments are never as descript as I would like them to be. 