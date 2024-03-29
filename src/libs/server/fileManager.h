#include <future>
#include <map>
#include <filesystem>
#include <string.h>

#include "../common/helpers.h"
#include "../common/message.h"

enum FileActionType
{
    Upload,
    Subscribe,
    Read,
    Delete,
    ListServer,
    Unsubscribe
};

class FileAction
{
public:
    Session session = Session(0, 0, "");
    std::string filename;
    FileActionType type;
    time_t timestamp;

    FileAction(Session _session,
               std::string _filename,
               FileActionType _type,
               time_t _timestamp)
    {
        filename = _filename;
        session = _session;
        type = _type;
        timestamp = _timestamp;
    }
};

enum FileStateTag
{
    EmptyFile,
    Reading,
    Updating,
    Deleting
};

class FileState
{
public:
    FileStateTag tag;
    std::future<void> *executingOperation;

    time_t created;
    time_t updated;
    time_t acessed;

    bool IsEmptyState() { return this->tag == FileStateTag::EmptyFile; }
    bool IsReadingState() { return this->tag == FileStateTag::Reading; }
    bool IsUpdatingState() { return this->tag == FileStateTag::Updating; }
    bool IsDeletingState() { return this->tag == FileStateTag::Deleting; }

    static FileState Empty()
    {
        FileState state;
        state.tag = FileStateTag::EmptyFile;
        return state;
    }
};

class UserFiles
{

public:
    std::map<std::string, FileState> fileStatesByFilename;
    std::list<int> *subscribers = new std::list<int>();

    FileState get(std::string filename)
    {
        if (fileStatesByFilename.find(filename) == fileStatesByFilename.end())
        {
            return FileState::Empty();
        }

        return fileStatesByFilename[filename];
    }

    void update(std::string name, FileState state)
    {
        fileStatesByFilename[name] = state;
    }
};

std::string extractLabelFromPath(std::string path);

class FilesManager
{
    std::map<std::string, UserFiles *> userFilesByUsername;

public:
    FilesManager()
    {
        for (const auto &userEntry : std::filesystem::directory_iterator("out/"))
        {
            if (!userEntry.is_directory())
            {
                continue;
            }

            std::string username = extractLabelFromPath(userEntry.path());

            UserFiles *userFiles = getFiles(username);

            for (const auto &fileEntry : std::filesystem::directory_iterator("out/" + username))
            {
                if (!fileEntry.is_regular_file())
                {
                    continue;
                }
                std::string path = fileEntry.path();
                std::string filename = extractLabelFromPath(path);

                FileState fileState = FileState::Empty();
                fileState.tag = FileStateTag::Updating;
                fileState.executingOperation = allocateFunction();
                *(fileState.executingOperation) = std::async(launch::async, [] {});

                fileState.acessed = getAccessTime(path);
                fileState.created = getCreateTime(path);
                fileState.updated = getModificationTime(path);

                userFiles->fileStatesByFilename[filename] = fileState;
            }
        }
    }

    UserFiles *getFiles(std::string username)
    {
        if (userFilesByUsername.find(username) == userFilesByUsername.end())
        {
            UserFiles *initial = (UserFiles *)malloc(sizeof(UserFiles));
            *initial = UserFiles();
            userFilesByUsername[username] = initial;
        }

        return userFilesByUsername[username];
    }
};

std::string toString(FileActionType type);
std::string fileActionToString(FileAction fileAction);
std::string toString(FileState fileState);

FileState getNextState(FileState lastFileState, FileAction fileAction, std::function<void(FileState)> onComplete);
