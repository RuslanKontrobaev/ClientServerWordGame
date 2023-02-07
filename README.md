# Client-Server Word Game

Console client-server application - a word game for three players, using the WinSock library and WinAPI functions

- Game interaction is implemented via the TCP protocol
- The search for servers is carried out by means of broadcast packets using the UDP protocol

After the server search is completed, a list of found servers is generated, each of which can be connected to

## Example of how the program works

### Starting the server
![image](https://user-images.githubusercontent.com/109802024/217182711-5b867466-4c7a-4424-b2ad-be97c504a21c.png)

### Client launch and server selection
![image](https://user-images.githubusercontent.com/109802024/217182861-9093a446-6a3a-45d9-9bc1-6a8ad79f8776.png)

### Connecting to the selected server
![image](https://user-images.githubusercontent.com/109802024/217183044-7720bcfd-b32a-4dc8-933e-0a60d1fa2695.png)

The game requires three players, so we expect the 2nd and 3rd player. After connecting the 2nd and 3rd player, each player will be shown the rules of the game. The interaction of the players is a word game in turn. The order is determined by the order of connection to the server. The first player writes a word (at least 2 letters), each next player must write a new word that will begin with the letter that ends the word of the previous player. Each player's entered word is sent to all other players, on the server side, the words of all players are also output to the console

### The console of the 1st client during the game
![image](https://user-images.githubusercontent.com/109802024/217185418-dea56ad4-5723-458c-8d8a-96cb9d96084e.png)

### The console of the 2nd client during the game
![image](https://user-images.githubusercontent.com/109802024/217183567-da4805d9-9157-4b25-aa64-59a8352dd282.png)

### The console of the 3rd client during the game
![image](https://user-images.githubusercontent.com/109802024/217183629-fac97a2e-ccc3-43d9-a7c9-ae687313eec4.png)

### Server console during the game
![image](https://user-images.githubusercontent.com/109802024/217185203-5084b8da-b3f9-49f2-963a-0573ace87893.png)
