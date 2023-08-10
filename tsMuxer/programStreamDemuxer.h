#ifndef PROGRAM_STREAM_DEMUXER_H_
#define PROGRAM_STREAM_DEMUXER_H_

#include "BufferedReader.h"
#include "abstractDemuxer.h"
#include "bufferedReaderManager.h"

class ProgramStreamDemuxer : public AbstractDemuxer
{
   public:
    static constexpr int MAX_PES_HEADER_SIZE = 1018;  // buffer for PES header and program stream map

    ProgramStreamDemuxer(BufferedReaderManager& readManager);
    void openFile(const std::string& streamName) override;
    virtual int readPacket(AVPacket& avPacket) { return 0; }
    ~ProgramStreamDemuxer() override;
    int simpleDemuxBlock(DemuxedData& demuxedData, const PIDSet& acceptedPIDs, int64_t& discardSize) override;
    void getTrackList(std::map<uint32_t, TrackInfo>& trackList) override;
    void readClose() override;
    uint64_t getDemuxedSize() override;
    int getLastReadRez() override { return m_lastReadRez; }
    void setFileIterator(FileNameIterator* itr) override;

    int64_t getTrackDelay(const uint32_t pid) override
    {
        if (m_firstPtsTime.find(pid) != m_firstPtsTime.end())
            return static_cast<int64_t>((m_firstPtsTime[pid] - (m_firstVideoPTS != -1 ? m_firstVideoPTS : m_firstPTS)) /
                                            90.0 +
                                        0.5);  // convert to ms

        return 0;
    }

    int64_t getFileDurationNano() const override;

   private:
    uint32_t m_tmpBufferLen;
    uint8_t m_tmpBuffer[MAX_PES_HEADER_SIZE];  // TS_FRAME_SIZE
    uint32_t m_lastPesLen;
    uint32_t m_lastPID;
    BufferedReaderManager& m_readManager;
    uint64_t m_dataProcessed;
    std::string m_streamName;
    int m_readerID;
    int m_lastReadRez;
    AbstractReader* m_bufferedReader;
    uint8_t m_psm_es_type[256];

    std::map<int, int64_t> m_firstPtsTime;
    int64_t m_firstVideoPTS;
    int64_t m_firstPTS;
    MemoryBlock m_lpcmWaveHeader[16];
    bool m_lpcpHeaderAdded[16];

   private:
    bool isVideoPID(uint32_t pid) const;
    int mpegps_psm_parse(const uint8_t* buff, const uint8_t* end);
    uint8_t processPES(uint8_t* buff, uint8_t* end, int& afterPesHeader);
};

#endif
