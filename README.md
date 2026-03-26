# HTTP Client - REST API Communication

This project is an implementation of an HTTP client that communicates with a REST API.

## Description

The application simulates a client-server interaction using HTTP. The server exposes a REST API, and the client (implemented in C/C++) sends requests and processes responses.

The client acts as a command-line interface for managing users, movies, and collections in an online library system. :contentReference[oaicite:0]{index=0}

## Features

- HTTP communication using sockets
- Interaction with a REST API
- JSON request and response handling
- Session management using cookies
- Authorization using JWT tokens
- Command-based interface

## Functionality

- Admin operations:
  - login/logout
  - add, view, and delete users
- User operations:
  - login
  - access library
- Movie management:
  - get, add, update, delete movies
- Collection management:
  - create and manage collections
- Handle server responses (success and errors)

## Commands

- login_admin
- add_user
- get_users
- delete_user
- logout_admin
- login
- get_access
- get_movies / get_movie
- add_movie / update_movie / delete_movie
- get_collections / get_collection
- add_collection / delete_collection
- add_movie_to_collection
- exit

## Technologies

- C / C++
- Sockets (TCP)
- HTTP protocol
- JSON parsing library

## How to run

Compile the client and run it from the terminal. Enter commands in the specified format, and the program will send HTTP requests to the server and display the responses.

## Notes

- The client communicates only through HTTP requests using sockets.
- Cookies and JWT tokens must be handled dynamically.
- The project focuses on understanding HTTP, REST APIs, and client-server communication.
