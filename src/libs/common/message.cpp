#include <fstream>

#include "message.h"
#include "helpers.h"

#define LOG_MESSAGES_SENT false

Message::Message()
{
    this->timestamp = now();
};

Message::Message(MessageType type)
{
    this->timestamp = now();
    this->type = type;
};

Message::Message(MessageType type, std::string filename)
{
    this->timestamp = now();
    this->type = type;
    this->filename = filename;
};

Message Message::InvalidMessage() { return Message(MessageType::InvalidMessage); }
Message Message::EndCommand() { return Message(MessageType::EndCommand); }
Message Message::ListServerCommand() { return Message(MessageType::ListServerCommand); }
Message Message::SubscribeUpdates() { return Message(MessageType::SubscribeUpdates); }
Message Message::Start() { return Message(MessageType::Start); }
Message Message::Empty() { return Message(MessageType::Empty); }

Message Message::DataMessage(std::string data)
{
    Message message(MessageType::DataMessage);
    message.data = data;
    return message;
}

Message Message::FileInfo(std::string filename, time_t mtime, time_t atime, time_t ctime)
{
    Message message(MessageType::FileInfo);
    message.filename = filename;
    message.mtime = mtime;
    message.atime = atime;
    message.ctime = ctime;
    return message;
}

Message Message::RemoteFileUpdate(std::string filename, time_t mtime, time_t atime, time_t ctime)
{
    Message message(MessageType::RemoteFileUpdate);
    message.filename = filename;
    message.mtime = mtime;
    message.atime = atime;
    message.ctime = ctime;
    return message;
}

Message Message::RemoteFileDelete(std::string filename, time_t mtime, time_t atime, time_t ctime)
{
    Message message(MessageType::RemoteFileDelete);
    message.filename = filename;
    message.mtime = mtime;
    message.atime = atime;
    message.ctime = ctime;
    return message;
}

Message Message::Response(ResponseType type)
{
    Message message(MessageType::Response);
    message.responseType = type;
    return message;
}

Message Message::Login(std::string username)
{
    Message message(MessageType::Login);
    message.username = username;
    return message;
}

Message Message::UploadCommand(std::string filename) { return Message(MessageType::UploadCommand, filename); }
Message Message::DownloadCommand(std::string filename) { return Message(MessageType::DownloadCommand, filename); }
Message Message::DeleteCommand(std::string filename) { return Message(MessageType::DeleteCommand, filename); }

bool isFileNameValid(std::string filename)
{
    if (filename.length() <= 0)
        return false;

    return true;
}

Message Message::Parse(char *_buffer)
{
    Message message;

    std::string buffer = _buffer;
    if (buffer.length() == 0)
    {
        return Message::Empty();
    }
    int headerSeparator = buffer.find(":");
    auto header = buffer.substr(0, headerSeparator).c_str();
    std::string data = buffer.length() <= headerSeparator ? "" : buffer.substr(headerSeparator + 1);

    MessageType messageType = (MessageType)atoi(header);

    switch (messageType)
    {
    case MessageType::FileInfo:
    {
        std::string format = "YYYY-MM-DD HH:mm:ss";
        int size = format.length();
        int spacer = size + 1;

        time_t mtime = toTimeT(data.substr(0 * spacer, size));
        time_t atime = toTimeT(data.substr(1 * spacer, size));
        time_t ctime = toTimeT(data.substr(2 * spacer, size));
        std::string filename = data.substr(3 * spacer);
        return Message::FileInfo(filename, mtime, atime, ctime);
    }

    case MessageType::RemoteFileUpdate:
    {
        std::string format = "YYYY-MM-DD HH:mm:ss";
        int size = format.length();
        int spacer = size + 1;

        time_t mtime = toTimeT(data.substr(0 * spacer, size));
        time_t atime = toTimeT(data.substr(1 * spacer, size));
        time_t ctime = toTimeT(data.substr(2 * spacer, size));
        std::string filename = data.substr(3 * spacer);
        return Message::RemoteFileUpdate(filename, mtime, atime, ctime);
    }

    case MessageType::RemoteFileDelete:
    {
        std::string format = "YYYY-MM-DD HH:mm:ss";
        int size = format.length();
        int spacer = size + 1;

        time_t mtime = toTimeT(data.substr(0 * spacer, size));
        time_t atime = toTimeT(data.substr(1 * spacer, size));
        time_t ctime = toTimeT(data.substr(2 * spacer, size));
        std::string filename = data.substr(3 * spacer);
        return Message::RemoteFileDelete(filename, mtime, atime, ctime);
    }

    case MessageType::Start:
    case MessageType::EndCommand:
    case MessageType::ListServerCommand:
    case MessageType::SubscribeUpdates:
    {
        return Message(messageType);
    }

    case MessageType::DataMessage:
    {
        return Message::DataMessage(data);
    }

    case MessageType::Login:
    {
        return Message::Login(data);
    }

    case MessageType::Response:
    {
        ResponseType responseType = (ResponseType)atoi(&data[0]);
        return Message::Response(responseType);
    }

    case MessageType::UploadCommand:
    case MessageType::DownloadCommand:
    case MessageType::DeleteCommand:
    {
        if (!isFileNameValid(data))
        {
            return Message::InvalidMessage();
        }

        return Message(messageType, data);
    }

    default:
    {
        std::cout << "Couldn't parse message: " << buffer << std::endl;
        std::cout << "Type: [" << messageType << "]" << std::endl;
        return Message::InvalidMessage();
    }
    }

    throw new std::exception;
}

std::string toString(MessageType messageType)
{
    switch (messageType)
    {
    case MessageType::Empty:
        return "Empty";
    case MessageType::InvalidMessage:
        return "InvalidMessage";
    case MessageType::Login:
        return "Login";
    case MessageType::UploadCommand:
        return "UploadCommand";
    case MessageType::RemoteFileUpdate:
        return "RemoteFileUpdate";
    case MessageType::RemoteFileDelete:
        return "RemoteFileDelete";
    case MessageType::DownloadCommand:
        return "DownloadCommand";
    case MessageType::DeleteCommand:
        return "DeleteCommand";
    case MessageType::EndCommand:
        return "EndCommand";
    case MessageType::ListServerCommand:
        return "ListServerCommand";
    case MessageType::SubscribeUpdates:
        return "SubscribeUpdates";
    case MessageType::FileInfo:
        return "FileInfo";
    case MessageType::DataMessage:
        return "DataMessage";
    case MessageType::Response:
        return "Response";
    case MessageType::Start:
        return "Start";
    }

    return "MESSAGE TYPE NOT HANDLED";
}

std::string toString(ResponseType responseType)
{
    switch (responseType)
    {
    case ResponseType::Ok:
        return "OK";
    case ResponseType::Invalid:
        return "Invalid";
    case ResponseType::FileNotFound:
        return "FileNotFound";
    }

    return "RESPONSE TYPE NOT HANDLED";
}

std::string toString(Message *message)
{
    if (message->type == MessageType::Response)
    {
        return toString(message->type) + " " + toString(message->responseType);
    };

    return toString(message->type);
}

enum MessageDirection
{
    SEND,
    RECEIVE
};

void logMessage(Message *message, MessageDirection direction)
{
    if (!LOG_MESSAGES_SENT)
    {
        return;
    }

    std::cout << "[" << toHHMMSS(now()) << "] ";

    if (direction == MessageDirection::SEND)
    {
        std::cout << Color::green << "> " << toString(message) << Color::reset << std::endl;
        return;
    }

    if (direction == MessageDirection::RECEIVE)
    {
        std::cout << Color::blue << "< " << toString(message) << Color::reset << std::endl;
        return;
    }

    std::cout << "DIRECTION NOT HANDLED" << std::endl;
}

Message Message::Listen(int socket)
{
    char buffer[MAX_BUFFER_SIZE];
    listenPacket(&buffer, socket);
    Message message = Message::Parse(buffer);
    message.socket = socket;

    logMessage(&message, MessageDirection::RECEIVE);

    return message;
}

std::string Message::toPacket()
{
    if (type == MessageType::Empty)
    {
        return "";
    };

    std::ostringstream packet;

    packet << type << ":";

    switch (type)
    {
    case MessageType::Login:
        packet << this->username;
        break;

    case MessageType::UploadCommand:
    case MessageType::DownloadCommand:
    case MessageType::DeleteCommand:
        packet << this->filename;
        break;

    case MessageType::Response:
        packet << this->responseType;
        break;

    case MessageType::DataMessage:
        packet << this->data;
        break;

    case MessageType::Start:
    case MessageType::EndCommand:
    case MessageType::ListServerCommand:
    case MessageType::InvalidMessage:
    case MessageType::SubscribeUpdates:
        break;

    case MessageType::RemoteFileUpdate:
    case MessageType::RemoteFileDelete:
    case MessageType::FileInfo:
        packet
            << toString(this->mtime) << ":"
            << toString(this->atime) << ":"
            << toString(this->ctime) << ":"
            << this->filename;
        break;
    }

    return packet.str();
}

Message Message::Reply(Message message, bool expectReply)
{
    logMessage(&message, MessageDirection::SEND);

    sendPacket(this->socket, message.toPacket());

    if (!expectReply)
        return Message::InvalidMessage();

    Message response = Message::Listen(this->socket);
    return response;
}

Message Message::send(int socket, bool expectReply)
{
    logMessage(this, MessageDirection::SEND);

    this->socket = socket;
    sendPacket(socket, this->toPacket());

    if (!expectReply)
        return Message::InvalidMessage();

    Message response = Message::Listen(this->socket);
    return response;
}

Message listenMessage(int socket)
{
    char buffer[MAX_BUFFER_SIZE];
    listenPacket(&buffer, socket);
    Message message = Message::Parse(buffer);
    message.socket = socket;
    return message;
}

void Message::panic()
{
    cout << Color::red
         << "Invalid server response.\n"
         << "Received: { " << std::endl
         << "type: " << this->type << std::endl
         << "data: " << this->data << std::endl
         << "responseType: " << this->responseType << std::endl
         << "filename: " << this->filename << std::endl
         << "username: " << this->username << std::endl
         << "}"
         << Color::reset
         << endl;
}

bool Message::isOk()
{
    return this->type == MessageType::Response &&
           this->responseType == ResponseType::Ok;
}

// == FILE ============================================

void deleteFile(Session session, string path)
{
    Message message = Message::Listen(session.socket);
    if (message.type != MessageType::Start)
    {
        message.panic();
        return;
    }

    remove(path.c_str());
    message.Reply(Message::Response(ResponseType::Ok), false);
}

void downloadFile(Session session, string temporaryPath, string finalPath)
{
    std::fstream file;
    file.open(temporaryPath, ios::out);

    Message message = Message::Listen(session.socket);

    if (message.type != MessageType::Start)
    {
        message.panic();
        return;
    }

    message = message.Reply(Message::Response(ResponseType::Ok));

    while (true)
    {
        if (message.type == MessageType::DataMessage)
        {
            file << message.data;
            message = message.Reply(Message::Response(ResponseType::Ok));
            continue;
        }

        if (message.type == MessageType::EndCommand)
        {
            message.Reply(Message::Response(ResponseType::Ok), false);
            break;
        }

        message.panic();
    }

    file.close();
    rename(temporaryPath.c_str(), finalPath.c_str());
}

void sendFile(Session session, string path)
{
    std::fstream file;
    file.open(path, ios::in);

    Message message = Message::Start().send(session.socket);

    if (!message.isOk())
    {
        message.panic();
        file.close();
        return;
    }

    char ch;
    string line;
    std::cout << "Sending file..." << std::endl;

    while (file >> noskipws >> ch)
    {
        line.push_back(ch);

        if (line.size() == 100)
        {
            message = message.Reply(Message::DataMessage(line));

            if (!message.isOk())
            {
                message.panic();
                file.close();
                return;
            }

            line.clear();
        }
    }

    if (line.size() > 0)
    {
        message = message.Reply(Message::DataMessage(line));

        if (!message.isOk())
        {
            message.panic();
            file.close();
            return;
        }

        line.clear();
    }

    std::cout << "OK!" << std::endl;

    message.Reply(Message::EndCommand());
    file.close();
}

ServerConnection::ServerConnection(char *serverIpAddress, int port, std::string username)
{
    this->serverIpAddress = serverIpAddress;
    this->port = port;
    this->username = username;
}

Message ServerConnection::connect()
{
    int socket = connectToServer(serverIpAddress, port);
    auto message = Message::Login(username).send(socket);

    if (!message.isOk())
    {
        message.panic();
        throw new std::exception();
    }

    return message;
}