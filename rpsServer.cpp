/****************************************************************************
* Program:
*    Lab RockSrvT2, Rock/Paper/Scissors with Sockets - Server Code
* Author:
*    Alex Johnson
* Summary:
*    This program implements a simple Rock Paper Scissors game over TCP using
*    sockets. This client program provides the interface with the RPS server.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <algorithm>
//  #include <signal.h>

#define PORT "8192"     // the port users will be connecting to
#define MAXDATASIZE 256 // max number of bytes we can get at once
#define BACKLOG 10      // how many pending connections queue will hold

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET)
   {
      return &(((struct sockaddr_in *)sa)->sin_addr);
   }

   return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void)
{
   int sockfd;        // listen on sock_fd
   int fd_player_one; // socket file descriptor for player one
   int fd_player_two; // socket file descriptor for player two
   struct addrinfo hints;
   struct addrinfo *servinfo;
   struct addrinfo *p;
   struct sockaddr_storage their_addr; // connector's address information
   socklen_t sin_size;
   //struct sigaction sa;
   int yes = 1;
   char s[INET6_ADDRSTRLEN];
   int rv;

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE; // use my IP

   if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
   {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }

   // loop through all the results and bind to the first we can
   for (p = servinfo; p != NULL; p = p->ai_next)
   {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
                           p->ai_protocol)) == -1)
      {
         perror("server: socket");
         continue;
      }

      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                     sizeof(int)) == -1)
      {
         perror("setsockopt");
         exit(1);
      }

      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
      {
         close(sockfd);
         perror("server: bind");
         continue;
      }

      break;
   }

   freeaddrinfo(servinfo); // all done with this structure

   if (p == NULL)
   {
      fprintf(stderr, "server: failed to bind\n");
      exit(1);
   }

   if (listen(sockfd, BACKLOG) == -1)
   {
      perror("listen");
      exit(1);
   }

   printf("server: waiting for connections...\n");

   bool keep_playing = true;
   bool already_playing = false;

   while (keep_playing)
   {
      std::string action_player_one = "none";
      char first_letter_player_one = 'n';

      std::string action_player_two = "none";
      char first_letter_player_two = 'n';

      // Buffer to hold bytestream
      char buffer[MAXDATASIZE];

      while (action_player_one == "none" && keep_playing == true) // main accept() loop
      {
         // receive loop for player one
         sin_size = sizeof their_addr;
         fd_player_one = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
         if (fd_player_one == -1)
         {
            perror("accept");
            continue;
         }

         inet_ntop(their_addr.ss_family,
                   get_in_addr((struct sockaddr *)&their_addr),
                   s, sizeof s);
         if (already_playing == false)
            printf("server: got connection from %s\n", s);

         // Get player one's action and record the number of bytes
         int numbytes = recv(fd_player_one, buffer, MAXDATASIZE - 1, 0);

         // Append null char to end of buffer
         buffer[numbytes] = '\0';

         // Store the buffer contents in the string
         action_player_one = buffer;

         // Convert string to lowercase for comparisons
         std::transform(action_player_one.begin(), action_player_one.end(),
                        action_player_one.begin(), ::tolower);

         // Display to console
         if (action_player_one.empty())
            std::cout << "\nPlayer One provided no input.\n";
         else
            std::cout << "\nPlayer One enters: " + action_player_one + '\n';

         // Check for quitters
         first_letter_player_one = action_player_one.at(0);
         if (first_letter_player_one == 'q')
         {
            std::cout << "\nPlayer One quits!\n";
            keep_playing = false;
         }
      }

      // receive loop for player two
      while (action_player_two == "none" && keep_playing == true) // main accept() loop
      {
         sin_size = sizeof their_addr;
         fd_player_two = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
         if (fd_player_two == -1)
         {
            perror("accept");
            continue;
         }

         inet_ntop(their_addr.ss_family,
                   get_in_addr((struct sockaddr *)&their_addr),
                   s, sizeof s);
         if (already_playing == false)
            printf("server: got connection from %s\n", s);

         // Buffer to hold bytestream from player one
         char buffer[MAXDATASIZE];

         // Get player two's action and record the number of bytes
         int numbytes = recv(fd_player_two, buffer, MAXDATASIZE - 1, 0);

         // Append null char to end of buffer
         buffer[numbytes] = '\0';

         // Store the buffer contents in the string
         action_player_two = buffer;

         // Convert string to lowercase for comparisons
         std::transform(action_player_two.begin(), action_player_two.end(),
                        action_player_two.begin(), ::tolower);

         // Display to console
         if (action_player_two.empty())
            std::cout << "\nPlayer Two provided no input.\n";
         else
            std::cout << "\nPlayer Two enters: " + action_player_two + '\n';

         // Check for quitters
         first_letter_player_two = action_player_two.at(0);
         if (first_letter_player_two == 'q')
         {
            std::cout << "\nPlayer Two quits!\n";
            keep_playing = false;
         }
      }

      std::string recap = "none";
      std::string result_player_one = "none";
      std::string result_player_two = "none";

      // Figure out the winner and the loser
      switch (first_letter_player_one)
      {
      case 'r':
         recap = "Player One played rock. ";
         switch (first_letter_player_two)
         {
         case 'r':
            recap += "Player Two played rock.\n";
            result_player_one, result_player_two = "It's a draw!\n";
            break;
         case 'p':
            recap += "Player Two played paper.\n";
            result_player_one = "Paper covers rock. You lose!\n";
            result_player_two = "Paper covers rock. You win!\n";
            break;
         case 's':
            recap += "Player Two played scissors.\n";
            result_player_one = "Rock smashes scissors. You win!\n";
            result_player_two = "Rock smashes scissors. You lose!\n";
            break;
         }
         break;
      case 'p':
         recap = "Player One played paper. ";
         switch (first_letter_player_two)
         {
         case 'r':
            recap += "Player Two played rock.\n";
            result_player_one = "Paper covers rock. You win!\n";
            result_player_two = "Paper covers rock. You lose!\n";
            break;
         case 'p':
            recap += "Player Two played paper.\n";
            result_player_one, result_player_two = "It's a draw!\n";
            break;
         case 's':
            recap += "Player Two played scissors.\n";
            result_player_one = "Scissors cuts paper. You lose!\n";
            result_player_two = "Scissors cuts paper. You win!\n";
            break;
         }
         break;
      case 's':
         recap = "Player One played scissors. ";
         switch (first_letter_player_two)
         {
         case 'r':
            recap += "Player Two played rock.\n";
            result_player_one = "Rock smashes scissors. You lose!\n";
            result_player_two = "Rock smashes scissors. You win!\n";
            break;
         case 'p':
            recap += "Player Two played paper.\n";
            result_player_one = "Scissors cuts paper. You win!\n";
            result_player_two = "Scissors cuts paper. You lose!\n";
            break;
         case 's':
            recap += "Player Two played scissors.\n";
            result_player_one, result_player_two = "It's a draw!\n";
            break;
         }
      }

      std::string response_player_one = recap + result_player_one;
      std::string response_player_two = recap + result_player_two;

      send(fd_player_one, response_player_one.c_str(), response_player_one.length(), 0);
      send(fd_player_two, response_player_two.c_str(), response_player_two.length(), 0);

      close(fd_player_one);
      close(fd_player_two);

      already_playing == true;
   }

   return 0;
}
