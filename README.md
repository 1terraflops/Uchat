# Uchat
Messaging application built with C

## 💡 About Uchat
Uchat is a GUI messaging application built with C, GTK, sqlite and CSS.

The project is divided into two executables: uchat and uchat_server.

On the frontend side, in order to build the GUI the project uses GTK. CSS is used for styling.

On the backend, the uchat_server is a daemonized process that runs in the background. The server uses sockets for data exchange between clients. To store users and their messaging history, uchat_server uses sqlite.

Uchat messenger is capable of:
* Registering/logging in users
* Searching for contacts
* Exchanging messages
* Emoji support
* Saving message history
* Automatically reconnecting if the connection was lost

## 🔧 Built with
* **C**
* **sqlite**
* **GTK3**
* **CSS**
* **Make**
* **libmx** (custom library)
* **Valgrind**

## 🚀 Getting started
These instructions will help you configure and compile this project to run it on your machine.

### 🧩 Prerequisites
Before trying to compile the application, make sure you have **clang compiler**, **Make** and **GTK3** installed on your system:

* On Linux (Ubuntu):
```
sudo apt install build-essential clang libgtk-3-dev
```

* On macOS:
```
xcode-select --install
```
```
brew install gtk+3
```

### ⚙️ Installation
After cloning the repository, navigate to Uchat root directory and simply run 'make' command:
```
make
```

The application should now be ready to run.

## ✅ Usage

Before launching the client, start `uchat_server` and specify a port. For example:
```
./uchat_server 3000
```

Then, start the client. Specify the ip address and port. For example:
```
./uchat 127.0.0.1 3000
```

**Preview:**

<img width="951" alt="image" src="https://github.com/user-attachments/assets/4909a015-05f2-439e-a704-89648b8a4367" />
