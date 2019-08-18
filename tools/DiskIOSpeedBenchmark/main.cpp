//
// Created by yuan on 8/5/19.
//

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <stdio.h>

#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <aio.h>
#include <linux/aio_abi.h>
#include <poll.h>
#include <sys/syscall.h>
#include <sys/uio.h>

#include <XmlPathFileReader.h>
#include <PatentTagTextCollector.h>
#include "XmlPCProcessorInterface.h"
#include "StatsThread.h"


using namespace std;
namespace fs = std::filesystem;

using OutputQueueByFile = unordered_map<string, vector<string>>;

inline size_t getFileSize(const char* filename)
{
    struct stat st;
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1;
}

class DoNothingWriter : public ThreadJob<CQueue<string>&> {
    vector<string> filenames_;

    void internalRun(CQueue<string>& data) final
    {
        for (;;)
        {
            auto [filename, quit] = data.pop();

            if (quit) break;

            filenames_.push_back(filename);
        }

        cout << filenames_.size() << '\n';
    }
};


class DoNothingCollector : public ThreadJob<> {
    CQueue<string>& filenameQueue_;

    void internalRun() final
    {
        for (;;)
        {
            pugi::xml_document doc;
            auto [filename, quit] = filenameQueue_.pop();

            if (quit) break;

            pugi::xml_parse_result result = doc.load_file(filename.c_str());
        }
    }
public:
    explicit DoNothingCollector(CQueue<string>& filenameQueue) : filenameQueue_(filenameQueue) { }
};


class DoNothingReader : public ThreadJob<> {
    CQueue<string>& filenameQueue_;

    void internalRun() final
    {
        for (;;)
        {
            auto [filename, quit] = filenameQueue_.pop();
            if (quit) break;
            ifstream ifs(filename);

            stringstream buffer;
            buffer << ifs.rdbuf();

            ifs.close();
        }
    }
public:
    explicit DoNothingReader(CQueue<string>& filenameQueue) : filenameQueue_(filenameQueue) { }
};


class DoNothingSyncCReader : public ThreadJob<> {
    CQueue<string>& filenameQueue_;

    inline static constexpr size_t DEFAULT_BUFSIZE = 1024;
    size_t bufferSize_ = DEFAULT_BUFSIZE;
    char* buffer_;

    void internalRun() final
    {
        for (;;)
        {
            auto [filename, quit] = filenameQueue_.pop();
            if (quit) break;

            FILE* inputFile = fopen(filename.c_str(), "r");
            if(!inputFile) {
                cerr << "File opening failed\n";
                continue;
            }
            /* Get the number of bytes */
            fseek(inputFile, 0L, SEEK_END);
            size_t numBytes = ftell(inputFile);

            fseek(inputFile, 0L, SEEK_SET);

            if (bufferSize_ < numBytes) {
                free(buffer_);
                bufferSize_ = numBytes;
                buffer_ = (char*)malloc(bufferSize_);
            }

            fread(buffer_, sizeof(char), numBytes, inputFile);

            fclose(inputFile);
        }
    }
public:
    explicit DoNothingSyncCReader(CQueue<string>& filenameQueue) : filenameQueue_(filenameQueue)
    {
        buffer_ = (char*)malloc(bufferSize_);
    }

    ~DoNothingSyncCReader() final
    {
        free(buffer_);
    }
};


class DoNothingAsyncReader : public ThreadJob<> {
    CQueue<string>& filenameQueue_;

    inline static constexpr size_t DEFAULT_BUFSIZE = 1024;
    size_t bufferSize_ = DEFAULT_BUFSIZE;
    char* buffer_;

    void internalRun() final
    {
        for (;;)
        {
            auto [filename, quit] = filenameQueue_.pop();
            if (quit) break;

            FILE* inputFile = fopen(filename.c_str(), "r");
            if(!inputFile) {
                cerr << "File opening failed\n";
                continue;
            }

            fseek(inputFile, 0L, SEEK_END);
            size_t numBytes = ftell(inputFile);

            fseek(inputFile, 0L, SEEK_SET);

            if (bufferSize_ < numBytes) {
                free(buffer_);
                bufferSize_ = numBytes;
                buffer_ = (char*)malloc(bufferSize_);
            }

            fread(buffer_, sizeof(char), numBytes, inputFile);

            fclose(inputFile);
        }
    }
public:
    explicit DoNothingAsyncReader(CQueue<string>& filenameQueue) :
        filenameQueue_(filenameQueue)
    {
        buffer_ = (char*)malloc(bufferSize_);
    }

    ~DoNothingAsyncReader() final
    {
        free(buffer_);
    }
};

class FileSizeReaderWithFopen : public ThreadJob<> {
    CQueue<string>& filenameQueue_;


    void internalRun() final
    {
        for (;;)
        {
            auto [filename, quit] = filenameQueue_.pop();
            if (quit) break;

            FILE* inputFile = fopen(filename.c_str(), "r");
            if(!inputFile) {
                cerr << "File opening failed\n";
                continue;
            }

            fseek(inputFile, 0L, SEEK_END);
            size_t numBytes = ftell(inputFile);

            fseek(inputFile, 0L, SEEK_SET);

            fclose(inputFile);
        }
    }
public:
    explicit FileSizeReaderWithFopen(CQueue<string>& filenameQueue) :
            filenameQueue_(filenameQueue)
    {
    }

    ~FileSizeReaderWithFopen() final
    {
    }
};


class FileSizeReaderWithStat : public ThreadJob<> {
    CQueue<string>& filenameQueue_;


    void internalRun() final
    {
        for (;;)
        {
            auto [filename, quit] = filenameQueue_.pop();
            if (quit) break;

            size_t fileSize = getFileSize(filename.c_str());
        }
    }
public:
    explicit FileSizeReaderWithStat(CQueue<string>& filenameQueue) :
            filenameQueue_(filenameQueue)
    {
    }
};


class DoNothingWithRead : public ThreadJob<> {
    CQueue<string>& filenameQueue_;

    inline static constexpr size_t DEFAULT_BUFSIZE = 1024;

    size_t bufferSize_;
    char* buffer_;
    aiocb cb_;

    void resetAiocb()
    {
        memset(&cb_, 0, sizeof(aiocb));
    }

    void internalRun() final
    {
        for (;;)
        {
            auto [filename, quit] = filenameQueue_.pop();
            if (quit) break;

            size_t fileSize = getFileSize(filename.c_str());

            if (bufferSize_ < fileSize) {
                bufferSize_ = fileSize * 2;
                delete[] buffer_;
                buffer_ = new char[bufferSize_];
            }

            int fd = open(filename.c_str(), O_RDONLY);
            if (fd == -1) {
                fprintf(stderr, "Cannot open [%s]\n", filename.c_str());
                continue;
            }

            resetAiocb();
            cb_.aio_nbytes = fileSize;
            cb_.aio_fildes = fd;
            cb_.aio_offset = 0;
            cb_.aio_buf = buffer_;

            if (aio_read(&cb_) == -1) {
                fprintf(stderr, "failed to create aio_read request for [%s]\n", filename.c_str());
                continue;
            }

            close(fd);
        }
    }
public:
    explicit DoNothingWithRead(CQueue<string>& filenameQueue) :
        filenameQueue_(filenameQueue), bufferSize_(DEFAULT_BUFSIZE),
        buffer_(new char[bufferSize_]) { resetAiocb(); }

    ~DoNothingWithRead() final
    {
        delete[] buffer_;
    }
};


class DiskIOSpeedBenchmark : public XmlPCProcessorInterface {
    string pathFilename1_;
    string pathFilename2_;

    CQueue<string> filenameQueue1_, filenameQueue2_;

    void prepareNodeFilters() final { }
    void prepareOutputFormatters() final { }
    void initializeData() final
    {
        XmlPathFileReader xmlFileReader(pathFilename1_, filenameQueue1_);
        XmlPathFileReader xmlFileReader2(pathFilename2_, filenameQueue2_);
        xmlFileReader.runOnMain();
        xmlFileReader2.runOnMain();
    }

    void initializeThreads() final
    {
        for (int i = 0; i < nProducers_; i++) {
            producers_.add<DoNothingReader>(filenameQueue1_);
            producers_.add<DoNothingReader>(filenameQueue2_);
        }
    }

    void executeThreads() final
    {
        producers_.runAll();
        producers_.waitAll();
    }

public:
    DiskIOSpeedBenchmark(string_view pathFilename1, string_view pathFilename2, int nFileReaders) :
        pathFilename1_(pathFilename1), pathFilename2_(pathFilename2)
    { nProducers_ = nFileReaders; }
};

void printUsageAndExit(const char* program)
{
    printf("Usage:\n\t\t%s <path-file-1> <path-file-2> <num-threads>\n", program);
    exit(-1);
}

/*
int main(int argc, char* argv[])
{
    if (argc != 4)
        printUsageAndExit(argv[0]);

    DiskIOSpeedBenchmark diskIoSpeedBenchmark(argv[1], argv[2], atoi(argv[3]));

    diskIoSpeedBenchmark.process();

    return 0;
}
*/

class PathFinder : public ThreadJob<CQueue<string>&> {

    vector<fs::path> pathRoots_;
    size_t batchSize_;

    void internalRun(CQueue<string>& filepathQueue) final
    {
        vector<string> fileBatch;
        fileBatch.reserve(batchSize_);

        for (const fs::path& root : pathRoots_) {
            if (!fs::exists(root))
                fprintf(stderr, "path [%s] does not exist\n", root.c_str());
            for (const fs::directory_entry& entry : fs::recursive_directory_iterator(root)) {
                if (fs::is_regular_file(entry.path())) {
                    fileBatch.push_back(entry.path());
                }

                if (fileBatch.size() == batchSize_) {
                    filepathQueue.push(fileBatch);

                    fileBatch.clear();
                }
            }
            if (!fileBatch.empty()) {
                filepathQueue.push(fileBatch);
                fileBatch.clear();
            }
        }
        filepathQueue.setQuitSignal();
    }
public:
    PathFinder(vector<fs::path>&& pathRoots, CQueue<string>& filepathQueue,
            size_t batchSize = 128) :
        ThreadJob(filepathQueue), pathRoots_(pathRoots), batchSize_(batchSize) { }
};

class DiskIOBenchmarkWithPathFinder : public XmlPCProcessorInterface {
    string pathRoot_;

    CQueue<string> filenameQueue_;

    void prepareNodeFilters() final { }
    void prepareOutputFormatters() final { }
    void initializeData() final
    {
        PathFinder pathFinder(
                {pathRoot_ + "/2015", pathRoot_ + "/2016"}, filenameQueue_);
        pathFinder.runOnMain();
    }

    void initializeThreads() final
    {
        for (int i = 0; i < nProducers_; i++) {
            producers_.add<DoNothingReader>(filenameQueue_);
        }
    }

    void executeThreads() final
    {
        producers_.runAll();
        producers_.waitAll();
    }

public:
    DiskIOBenchmarkWithPathFinder(string_view pathRoot, int nFileReaders) :
            pathRoot_(pathRoot)
    { nProducers_ = nFileReaders; }
};


class DiskIOBenchmarkWithCRead : public XmlPCProcessorInterface {
    string pathFilename_;

    CQueue<string> filenameQueue_;

    void prepareNodeFilters() final { }
    void prepareOutputFormatters() final { }
    void initializeData() final
    {
        XmlPathFileReader xmlFileReader(pathFilename_, filenameQueue_);

        xmlFileReader.runOnMain();
    }

    void initializeThreads() final
    {
        for (int i = 0; i < nProducers_; i++) {
            producers_.add<DoNothingWithRead>(filenameQueue_);
        }
    }

    void executeThreads() final
    {
        StatsThread<string, false> statsThread(filenameQueue_);
        producers_.runAll();
        statsThread.run();

        producers_.waitAll();
        statsThread.wait();
    }

public:
    DiskIOBenchmarkWithCRead(string_view pathFilename, int nFileReaders) :
            pathFilename_(pathFilename)
    { nProducers_ = nFileReaders; }
};


void printUsageAndExit2(const char* program)
{
    printf("Usage:\n\t\t%s <root-path> <num-threads>\n", program);
    exit(-1);
}

void singleLargeFileFreadBenchmark(const char* filename)
{
    FILE* inputFile = fopen(filename, "r");
    if(!inputFile) {
        cerr << "File opening failed\n";
        return;
    }
    /* Get the number of bytes */
    fseek(inputFile, 0L, SEEK_END);
    size_t numBytes = ftell(inputFile);

    fseek(inputFile, 0L, SEEK_SET);

    char* buffer = (char*)malloc(numBytes);

    fread(buffer, sizeof(char), numBytes, inputFile);

    fclose(inputFile);
}


void singleLargeFileReadBenchmark(const char* filename)
{
    size_t size = getFileSize(filename);

    cout << size << '\n';

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open [%s]\n", filename);
        exit(-1);
    }

    char* buffer = new char[size];

    size_t sizeRead;
    for (sizeRead = read(fd, buffer, size); sizeRead != 0; sizeRead = read(fd, buffer, size))
        cout << sizeRead << '\n';

    cout << sizeRead << '\n';

    close(fd);

    delete[] buffer;
}

void singleLargeFileAioReadBenchmark(const char* filename)
{
    size_t size = getFileSize(filename);

    cout << size << '\n';

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open [%s]\n", filename);
        exit(-1);
    }

    char* buffer = new char[size];

    aiocb cb;
    memset(&cb, 0, sizeof(aiocb));
    cb.aio_nbytes = size;
    cb.aio_fildes = fd;
    cb.aio_offset = 0;
    cb.aio_buf = buffer;

    if (aio_read(&cb) == -1) {
        fprintf(stderr, "aio_read failed on [%s]\n", filename);
        exit(-1);
    }

    while (aio_error(&cb) == EINPROGRESS);

//        cout << "waiting...\n";

    if (size_t sizeRead = aio_return(&cb); sizeRead != (size_t)-1)
        printf("Success: read %zu\n", sizeRead);
    else
        cout << "Failed\n";

    close(fd);

    delete[] buffer;
}

#ifndef IOCB_CMD_POLL
#define IOCB_CMD_POLL 5
#endif

inline static int io_setup(unsigned nr, aio_context_t *ctxp)
{
    return syscall(__NR_io_setup, nr, ctxp);
}

inline static int io_destroy(aio_context_t ctx)
{
    return syscall(__NR_io_destroy, ctx);
}

inline static int io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp)
{
    return syscall(__NR_io_submit, ctx, nr, iocbpp);
}


#define AIO_RING_MAGIC 0xa10a10a1
struct aio_ring {
    unsigned id; /** kernel internal index number */
    unsigned nr; /** number of io_events */
    unsigned head;
    unsigned tail;

    unsigned magic;
    unsigned compat_features;
    unsigned incompat_features;
    unsigned header_length; /** size of aio_ring */

    struct io_event events[0];
};

/* Stolen from kernel arch/x86_64.h */
#ifdef __x86_64__
#define read_barrier() __asm__ __volatile__("lfence" ::: "memory")
#else
#ifdef __i386__
#define read_barrier() __asm__ __volatile__("" : : : "memory")
#else
#define read_barrier() __sync_synchronize()
#endif
#endif

/* Code based on axboe/fio:
 * https://github.com/axboe/fio/blob/702906e9e3e03e9836421d5e5b5eaae3cd99d398/engines/libaio.c#L149-L172
 */
inline static int io_getevents(aio_context_t ctx, long min_nr, long max_nr,
                               struct io_event *events,
                               struct timespec *timeout)
{
    int i = 0;

    aio_ring *ring = (struct aio_ring *)ctx;
    if (ring == NULL || ring->magic != AIO_RING_MAGIC) {
        goto do_syscall;
    }

    while (i < max_nr) {
        unsigned head = ring->head;
        if (head == ring->tail) {
            /* There are no more completions */
            break;
        } else {
            /* There is another completion to reap */
            events[i] = ring->events[head];
            read_barrier();
            ring->head = (head + 1) % ring->nr;
            i++;
        }
    }

    if (i == 0 && timeout != NULL && timeout->tv_sec == 0 &&
        timeout->tv_nsec == 0) {
        /* Requested non blocking operation. */
        return 0;
    }

    if (i && i >= min_nr) {
        return i;
    }

    do_syscall:
    return syscall(__NR_io_getevents, ctx, min_nr - i, max_nr - i,
                   &events[i], timeout);
}

#define PFATAL(x...)                                                    \
	do {                                                                \
		fprintf(stderr, "[-] SYSTEM ERROR : " x);                       \
		fprintf(stderr, "\n\tLocation : %s(), %s:%u\n", __FUNCTION__,   \
			__FILE__, __LINE__);                                        \
		perror("      OS message ");                                    \
		fprintf(stderr, "%d\n", errno, EAGAIN);                                          \
		exit(EXIT_FAILURE);                                             \
	} while (0)


void singleLargeFileIoSubmitSingleBenchmark(const char* filename)
{
    size_t size = getFileSize(filename);

    cout << size << '\n';

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open [%s]\n", filename);
        exit(-1);
    }

    char* buffer = new char[size];

    iocb cb1;
    memset(&cb1, 0, sizeof(iocb));
    cb1.aio_nbytes = size;
    cb1.aio_fildes = fd;
    cb1.aio_lio_opcode = IOCB_CMD_PREAD;
    cb1.aio_buf = (__u64)buffer;

    iocb* iocbList[1] = {&cb1};

    aio_context_t ctx = 0;
    int ret = io_setup(128, &ctx);
    if (ret < 0)
        PFATAL("io_setup()");

    ret = io_submit(ctx, 1, iocbList);
    if (ret != 1)
        PFATAL("io_submit()");

    io_event events[1] = {};
    ret = io_getevents(ctx, 1, 1, events, NULL);
    if (ret != 1)
        PFATAL("io_getevents()");

    io_destroy(ctx);
    close(fd);

    delete[] buffer;
}


class FileInfo {
    inline static constexpr size_t INIT_BUF_SIZE = 1 << 15;

    string filename_ = "";
    size_t size_ = INIT_BUF_SIZE;
    iocb cb_;
    char* buffer_;

public:
    FileInfo() : buffer_(new char[size_ + 1]) { }

    explicit FileInfo(string_view filename) : buffer_(new char[size_ + 1])
    {
        loadNewFile(filename);
    }

    void loadNewFile(string_view filename)
    {
        if (!filename_.empty())
            close(cb_.aio_fildes);
        if (filename_ == filename)
            return;

        filename_ = filename;

        /* measure file size and allocate corresponding buffer */
        size_t oldSize = size_;
        size_ = getFileSize(filename_.c_str());
        if (oldSize < size_) {
            delete[] buffer_;
            buffer_ = new char[size_ + 1];
            buffer_[size_] = 0;
        }

        /* initialize iocb object */
        memset(&cb_, 0, sizeof(decltype(cb_)));
        cb_.aio_nbytes = size_;
        cb_.aio_fildes = open(filename_.c_str(), O_RDONLY);

        if ((ssize_t)cb_.aio_fildes == -1) {
            fprintf(stderr, "Cannot open [%s]\n", filename_.c_str());
            PFATAL("open()");
        }

        cb_.aio_lio_opcode = IOCB_CMD_PREAD;
        cb_.aio_buf = (size_t)buffer_;
    }

    iocb* getIocbPtr() { return &cb_; }

    ~FileInfo()
    {
        close(cb_.aio_fildes);
        delete[] buffer_;
    }
};


void multipleFileIoSubmitBenchmark(CQueue<string>& pathFileQueue)
{
    constexpr int nFilesToRead = 1 << 13;

    aio_context_t ctx = 0;
    int ret = io_setup(nFilesToRead, &ctx);
    io_event events[nFilesToRead] = {};
    if (ret < 0)
        PFATAL("io_setup()");

    vector<FileInfo> fileInfos(nFilesToRead);
//    FileInfo fileInfos[nFilesToRead] = { };
    iocb** iocbList = new iocb*[nFilesToRead];
    int i = 0;
    for (;;)
    {
        auto [filename, quit] = pathFileQueue.pop();

        if (quit) break;

        if (i >= nFilesToRead) {
            ret = io_submit(ctx, nFilesToRead, iocbList);
            if (ret != nFilesToRead) {
                fprintf(stderr, "%d\n", ret);
                PFATAL("io_submit()");
            }
            ret = io_getevents(ctx, nFilesToRead, nFilesToRead, events, nullptr);
            if (ret != nFilesToRead) {
                PFATAL("io_getevents()");
            }
            i = 0;
        }

        fileInfos[i].loadNewFile(filename);
        iocbList[i] = fileInfos[i].getIocbPtr();
        i++;
    }

    io_destroy(ctx);

    delete[] iocbList;
}


inline void queueFileForRead(const char* filename)
{
    size_t size = getFileSize(filename);

    cout << size << '\n';

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open [%s]\n", filename);
        exit(-1);
    }

    char* buffer = new char[size];

    iocb cb1;
    memset(&cb1, 0, sizeof(iocb));
    cb1.aio_nbytes = size;
    cb1.aio_fildes = fd;
    cb1.aio_lio_opcode = IOCB_CMD_PREAD;
    cb1.aio_buf = (__u64)buffer;

    iocb* iocbList[1] = {&cb1};

    aio_context_t ctx = 0;
    int ret = io_setup(128, &ctx);
    if (ret < 0)
        PFATAL("io_setup()");

    ret = io_submit(ctx, 1, iocbList);
    if (ret != 1)
        PFATAL("io_submit()");

    io_event events[1] = {};
    ret = io_getevents(ctx, 1, 1, events, NULL);
    if (ret != 1)
        PFATAL("io_getevents()");

    io_destroy(ctx);
    close(fd);

    delete[] buffer;
}


template <typename RequestType>
class File {
    inline static constexpr size_t INIT_BUF_SIZE = 1 << 15;

    const char*     filename_;
    int             fd_;
    size_t          size_;
    char*           buffer_;
    RequestType     request_;

    bool            isOpen_ = false;

public:
    enum class IoType { R, W, RW };

    File() : size_(INIT_BUF_SIZE), buffer_(new char[size_]) { }

    template <typename InitRequest>
    File(const char* filename, IoType ioType, InitRequest&& initRequest)
    {
        if (!open(filename, ioType, std::forward<InitRequest>(initRequest)))
            throw std::exception();
    }

    template <typename InitRequest>
    bool open(const char* filename, IoType ioType, InitRequest&& initRequest)
    {
        static_assert(is_invocable_v<InitRequest, RequestType&, decltype(this)>,
                      "InitRequest be of type T(*)(RequestType&)");
        filename_ = filename;

        int flag;
        switch (ioType)
        {
        case IoType::R:
            size_ = getFileSize(filename_);
            flag = O_RDONLY;
            break;
        case IoType::W:
            size_ = INIT_BUF_SIZE;
            flag = O_WRONLY;
            break;
        case IoType::RW:
            size_ = INIT_BUF_SIZE;
            flag = O_RDWR;
            break;
        default:
            fprintf(stderr, "wrong argument\n");
            return false;
        }

        fd_ = open(filename_, flag);
        if (fd_ == -1) {
            fprintf(stderr, "%s: failed to open [%s]\n", __FUNCTION__, filename_);

            return false;
        }

        buffer_ = new char[size_];
        initRequest(request_, this);

        isOpen_ = true;
        return true;
    }

    inline bool isOpen() const { return isOpen_; }

    ~File() { delete[] buffer_; }
};


/*
template <int n, typename RequestType>
class FileCollection {
    File<RequestType> files_[n];
    RequestType requestType_[n];
    int filesOpened_ = 0;
public:
    FileCollection() = default;

    template <typename InitRequest>
    bool openFile(const char* filename, typename File<RequestType>::IoType ioType,
            InitRequest&& initRequest)
    {
        if (filesOpened_ >= n)
            return false;

    }

    void readAll();
};
*/

template <int n>
class FileCollection {
    inline static constexpr size_t INIT_BUF_SIZE = 1 << 15;

    char* filename_[n];
    int fd_[n];
    size_t fileSize_[n];
    char* buffer_[n];
    size_t bufferSize_[n];
    iovec request_[n];
    bool filled_[n];

public:
    FileCollection()
    {
        fill_n(filled_, n, false);
        fill_n(fileSize_, n, 0);
        fill_n(bufferSize_, n, INIT_BUF_SIZE);
        for (int i = 0; i < n; i++)
            buffer_[i] = new char[bufferSize_[i]];
    }

    bool openFile(int i, const char* filename)
    {
        if (i >= n)
            throw range_error("FileCollection::openFile() i out of range");

        filename_[i] = filename;
        fileSize_[i] = getFileSize(filename);
        fd_[i] = open(filename, O_RDONLY);
        if (fd_[i] == -1) {
            fprintf(stderr, "%s: failed to open [%s]\n", __FUNCTION__, filename_);
            filled_[i] = false;
            return false;
        }
        if (bufferSize_[i] < fileSize_[i]) {
            delete[] buffer_[i];
            bufferSize_[i] <<= 1;
            buffer_[i] = new char[bufferSize_[i]];
        }
        request_[i] = {
                .iov_base = buffer_[i],
                .iov_len = fileSize_[i]
        };
        filled_[i] = true;

        return true;
    }

    bool readAll()
    {
        if (!all_of(filled_, filled_ + n, [](bool b) { return b; })) {
            fprintf(stderr, "%s: not all files are open\n", __FUNCTION__);
            return false;
        }
    }

    ~FileCollection()
    {
        for (int i = 0; i < n; i++)
            delete[] buffer_[i];
    }
};


void singleLargeFileReadvBenchmark(const char* filename)
{
    size_t size = getFileSize(filename);

    cout << size << '\n';

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open[%s]\n", filename);
        exit(-1);
    }

    char* buffer = new char[size];

    iovec iov = {
            .iov_base = buffer,
            .iov_len = size
    };

    ssize_t sizeRead = readv(fd, &iov, 1);
    if (sizeRead == -1)
        PFATAL("readv()");

    cout << sizeRead << '\n';

    close(fd);
    delete[] buffer;
}


int main(int argc, char* argv[])
{
/*
    if (argc != 3)
        printUsageAndExit2(argv[0]);

    DiskIOBenchmarkWithCRead diskIoBenchmarkWithCRead(argv[1], atoi(argv[2]));

    diskIoBenchmarkWithCRead.process();
*/
    singleLargeFileFreadBenchmark(argv[1]);

/*
    ifstream ifs(argv[1]);
    if (!ifs.is_open())
        PERROR("ifstream");
    stringstream ss;
    ss << ifs.rdbuf();
    ifs.close();
*/

/*
    CQueue<string> filenameQueue;
    XmlPathFileReader xmlFileReader(argv[1], filenameQueue);

    xmlFileReader.runOnMain();

    multipleFileIoSubmitBenchmark(filenameQueue);
*/
//    singleLargeFileReadvBenchmark(argv[1]);

    return 0;
}
