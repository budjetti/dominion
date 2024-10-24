# Overview
This c++ program is a command-line adaptation of base game Dominion (2008). Features bots with rudimentary strategy who love to attack.

# Building from source
In Linux or WSL on Windows, run the following commands. It's likely you already have g++ (or another compiler like gcc) installed. If not, you may need to install it with your package manager.

git clone https://github.com/budjetti/dominion  
cd dominion  
g++ dominion.cpp -o dominion  
./dominion

# FAQ
### Why was this made?
I was really into both Dominion and c++ at the time.
### Why is it all in one file?
Because I was lazy and didn't want to write header files.
