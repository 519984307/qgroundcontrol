/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
#include <cmath>

#include "LogDownloadController.h"
#include "MultiVehicleManager.h"
#include "QGCMAVLink.h"
#include "UAS.h"
#include "QGCApplication.h"
#include "QGCToolbox.h"
#include "QGCMapEngine.h"
#include "ParameterManager.h"
#include "Vehicle.h"
#include "SettingsManager.h"

#include <QDebug>
#include <QSettings>
#include <QUrl>
#include <QBitArray>
#include <QtCore/qmath.h>

#define kTimeOutMilliseconds 500
#define kGUIRateMilliseconds 17
#define kTableBins           512
#define kChunkSize           (kTableBins * MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN)

QGC_LOGGING_CATEGORY(LogDownloadLog, "LogDownloadLog")

//-----------------------------------------------------------------------------
struct LogDownloadData {
    LogDownloadData(QGCLogEntry* entry);
    QBitArray     chunk_table;
    uint32_t      current_chunk;
    QFile         file;
    QString       filename;
    uint          ID;
    QGCLogEntry*  entry;
    uint          written;
    size_t        rate_bytes;
    qreal         rate_avg;
    QElapsedTimer elapsed;

    void advanceChunk()
    {
           current_chunk++;
           chunk_table = QBitArray(chunkBins(), false);
    }

    // The number of MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN bins in the current chunk
    uint32_t chunkBins() const
    {
        return qMin(qCeil((entry->size() - current_chunk*kChunkSize)/static_cast<qreal>(MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN)),
                    kTableBins);
    }

    // The number of kChunkSize chunks in the file
    uint32_t numChunks() const
    {
        return qCeil(entry->size() / static_cast<qreal>(kChunkSize));
    }

    // True if all bins in the chunk have been set to val
    bool chunkEquals(const bool val) const
    {
        return chunk_table == QBitArray(chunk_table.size(), val);
    }

};

//----------------------------------------------------------------------------------------
LogDownloadData::LogDownloadData(QGCLogEntry* entry_)
    : ID(entry_->id())
    , entry(entry_)
    , written(0)
    , rate_bytes(0)
    , rate_avg(0)
{

}

//----------------------------------------------------------------------------------------
QGCLogEntry::QGCLogEntry(uint logId, const QDateTime& dateTime, uint logSize, bool received)
    : _logID(logId)
    , _logSize(logSize)
    , _logTimeUTC(dateTime)
    , _received(received)
    , _transferred(false)
    , _selected(false)
{
    _status = tr("Pending");
}

//----------------------------------------------------------------------------------------
QString
QGCLogEntry::sizeStr() const
{
    return QGCMapEngine::bigSizeToString(_logSize);
}

//----------------------------------------------------------------------------------------
LogDownloadController::LogDownloadController(void)
    : _uas(nullptr)
    , _downloadData(nullptr)
    , _vehicle(nullptr)
    , _requestingLogEntries(false)
    , _downloadingLogs(false)
    , _transferingLogs(false)
    , _completeTransfer(false)
    , _retries(0)
    , _apmOneBased(0)
{
    MultiVehicleManager *manager = qgcApp()->toolbox()->multiVehicleManager();
    connect(manager, &MultiVehicleManager::activeVehicleChanged, this, &LogDownloadController::_setActiveVehicle);
    connect(&_timer, &QTimer::timeout, this, &LogDownloadController::_processDownload);
    _setActiveVehicle(manager->activeVehicle());
    _since_start_timer.start(); // start timer for mavlink message timestamping
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_processDownload()
{
    //-- make sure that we don't run anything here if we are in _transferingLogs
    //-- requesting specific log entries or log data would mess up MAVSDK
    if(_requestingLogEntries && !_transferingLogs) {
        _findMissingEntries();
    } else if(_downloadingLogs && !_transferingLogs) {
        _findMissingData();
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_setActiveVehicle(Vehicle* vehicle)
{
    if(_uas) {
        _logEntriesModel.clear();
        disconnect(_uas, &UASInterface::logEntry, this, &LogDownloadController::_logEntry);
        disconnect(_uas, &UASInterface::logData,  this, &LogDownloadController::_logData);
        disconnect(_uas, &UASInterface::logCmp, this, &LogDownloadController::_logCmp);
        _uas = nullptr;
    }
    _vehicle = vehicle;
    if(_vehicle) {
        _uas = vehicle->uas();
        connect(_uas, &UASInterface::logEntry, this, &LogDownloadController::_logEntry);
        connect(_uas, &UASInterface::logData,  this, &LogDownloadController::_logData);
        connect(_uas, &UASInterface::logCmp, this, &LogDownloadController::_logCmp);
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_logEntry(UASInterface* uas, uint32_t time_utc, uint32_t size, uint16_t id, uint16_t num_logs, uint16_t /*last_log_num*/)
{
    //-- Do we care?
    if(!_uas || uas != _uas || !_requestingLogEntries) {
        return;
    }
    //-- If this is the first, pre-fill it
    if(!_logEntriesModel.count() && num_logs > 0) {
        //-- Is this APM? They send a first entry with bogus ID and only the
        //   count is valid. From now on, all entries are 1-based.
        if(_vehicle->firmwareType() == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            _apmOneBased = 1;
        }
        for(int i = 0; i < num_logs; i++) {
            QGCLogEntry *entry = new QGCLogEntry(i);
            _logEntriesModel.append(entry);
        }
    }
    //-- Update this log record
    if(num_logs > 0) {
        //-- Skip if empty (APM first packet)
        if(size || _vehicle->firmwareType() != MAV_AUTOPILOT_ARDUPILOTMEGA) {
            id -= _apmOneBased;
            if(id < _logEntriesModel.count()) {
                QGCLogEntry* entry = _logEntriesModel[id];
                entry->setSize(size);
                entry->setTime(QDateTime::fromTime_t(time_utc));
                entry->setReceived(true);
                if(_completeTransfer) {
                    entry->setStatus(tr("Waiting"));
                }
                else {
                    entry->setStatus(tr("Available"));
                }
            } else {
                qWarning() << "Received log entry for out-of-bound index:" << id;
            }
        }
    } else {
        //-- No logs to list
        _receivedAllEntries();
    }
    //-- Reset retry count
    _retries = 0;
    //-- Do we have it all?
    if(_entriesComplete()) {
        _receivedAllEntries();
    } else if(!_transferingLogs) {
        //-- Reset timer
        _timer.start(kTimeOutMilliseconds);
    }
}

//----------------------------------------------------------------------------------------
bool
LogDownloadController::_entriesComplete()
{
    //-- Iterate entries and look for a gap
    int num_logs = _logEntriesModel.count();
    for(int i = 0; i < num_logs; i++) {
        QGCLogEntry* entry = _logEntriesModel[i];
        if(entry) {
            if(!entry->received()) {
               return false;
            }
        }
    }
    return true;
}

//----------------------------------------------------------------------------------------
bool
LogDownloadController::_entriesTransferred()
{
    //-- Iterate entries and look for a gap
    int num_logs = _logEntriesModel.count();
    for(int i = 0; i < num_logs; i++) {
        QGCLogEntry* entry = _logEntriesModel[i];
        if(entry) {
            if(!entry->transferred()) {
               return false;
            }
        }
    }
    return true;
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_resetSelection(bool canceled)
{
    int num_logs = _logEntriesModel.count();
    for(int i = 0; i < num_logs; i++) {
        QGCLogEntry* entry = _logEntriesModel[i];
        if(entry) {
            if(entry->selected()) {
                if(canceled) {
                    entry->setStatus(tr("Canceled"));
                }
                entry->setSelected(false);
            }
        }
    }
    emit selectionChanged();
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_receivedAllEntries()
{
    _timer.stop();
    _setListing(false);
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_findMissingEntries()
{
    int start = -1;
    int end   = -1;
    int num_logs = _logEntriesModel.count();
    //-- Iterate entries and look for a gap
    for(int i = 0; i < num_logs; i++) {
        QGCLogEntry* entry = _logEntriesModel[i];
        if(entry) {
            if(!entry->received()) {
                if(start < 0)
                    start = i;
                else
                    end = i;
            } else {
                if(start >= 0) {
                    break;
                }
            }
        }
    }
    //-- Is there something missing?
    if(start >= 0) {
        //-- Have we tried too many times?
        if(_retries++ > 2) {
            for(int i = 0; i < num_logs; i++) {
                QGCLogEntry* entry = _logEntriesModel[i];
                if(entry) {
                    if(!entry->received()) {
                        entry->setStatus(tr("Error"));
                    }
                }
            }
            //-- Give up
            _receivedAllEntries();
            qWarning() << "Too many errors retreiving log list. Giving up.";
            return;
        }
        //-- Is it a sequence or just one entry?
        if(end < 0) {
            end = start;
        }
        //-- APM "Fix"
        start += _apmOneBased;
        end   += _apmOneBased;
        //-- Request these entries again
        _requestLogList((uint32_t)start, (uint32_t) end);
    } else {
        _receivedAllEntries();
    }
}

//----------------------------------------------------------------------------------------
void LogDownloadController::_updateDataRate(void)
{
    if (!_downloadData) {
        //-- Make sure we have download data to update
        return;
    }
    if (_downloadData->elapsed.elapsed() >= kGUIRateMilliseconds) {
        //-- Update download rate
        qreal rrate = _downloadData->rate_bytes / (_downloadData->elapsed.elapsed() / 1000.0);
        _downloadData->rate_avg = (_downloadData->rate_avg * 0.95) + (rrate * 0.05);
        _downloadData->rate_bytes = 0;

        //-- Update status
        const QString status = QString("%1 (%2/s)").arg(QGCMapEngine::bigSizeToString(_downloadData->written),
                                                        QGCMapEngine::bigSizeToString(_downloadData->rate_avg));

        _downloadData->entry->setStatus(status);
        _downloadData->elapsed.start();
    }
}

//----------------------------------------------------------------------------------------
void LogDownloadController::_logCmp(UASInterface* uas, uint64_t time_usec, uint8_t log_status)
{
    if(!_transferingLogs || !_downloadData) {
        //-- don't react to unsolicited status messages when we're not even
        //-- actively transfering logs
        return;
    }

    //-- float value is in format id.result (e.g. 5.3 -> id: 5, result 3)
    int id = _downloadData->ID;

    QGCLogEntry* entry = _getEntryByLogID(id);
    if(!entry) {
        //-- id was not found in our log list, can't do anything with this msg
        qWarning() << "No entry found with id" << id;
        return;
    }

    switch(log_status) {
        case LOG_STATUS::LOG_TRANSFER_FINISHED:
            entry->setStatus("Transferred");
            break;
        case LOG_STATUS::LOG_TRANSFER_FAILED:
            entry->setStatus("Failed");
            break;
        case LOG_STATUS::LOG_UPLOADING:
            entry->setStatus("Uploading");
            break;
        case LOG_STATUS::LOG_EXISTS:
            entry->setStatus("Already exists");
        default:
            //-- Don't do anything if we don't recognize the result
            break;
    }

    if (log_status != LOG_STATUS::LOG_UPLOADING) //-- Don't move on if we are still uploading
    {
        entry->setSelected(false);
        emit selectionChanged();
        entry->setTransferred(true);
        if(_downloadData && _downloadData->entry == entry) {
            //-- if the current downloadData is for the entry we received the
            //-- result for we can delete it since this download is complete now
            delete _downloadData;
            _downloadData = nullptr;
        }

        if(!_completeTransfer)
        {
            QGCLogEntry* next_entry = _getNextSelected();
            if(next_entry) {
                //-- if there is another selected entry, ask for its transfer
                _downloadData = new LogDownloadData(next_entry);
                _downloadData->elapsed.start();
                //-- It's stupid and I don't like it but mavlink can't handle if
                //-- we send back a NAMED_VALUE_FLOAT immediately so we wait 1s
                while(_downloadData->elapsed.elapsed() < 1000) {
                    continue;
                }
                //-- send request to transfer next file
                _sendLogTransferRequest(next_entry->id());
            }
            else
            {
                //-- no more logs, we are done
                _setTransfering(false);
            }
        } else if(_entriesTransferred()) //-- check if all entries were transferred
        {
            _setTransfering(false);
        }
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_logData(UASInterface* uas, uint32_t ofs, uint16_t id, uint8_t count, const uint8_t* data)
{
    if(!_uas || uas != _uas || (!_downloadData && !_transferingLogs)) {
        return;
    }

    if(_transferingLogs) {
        //-- if we are transfering logs, we don't care about data integrity and all the logic below,
        //-- just use the incoming log data to update the status (transferred size and transfer rate)
        if(_requestingLogEntries) {
            //-- if its the first log data message, we can stop listening for log entries
            _receivedAllEntries();
        }
        if(!_downloadData || _downloadData->ID != id) {
            if (_downloadData) {
                //-- _downloadData already exists meaning we transferred a log file before and
                //-- switched to the next file now without getting a message
                _downloadData->entry->setStatus(tr("Transferred"));
                _downloadData->entry->setTransferred(true);
                _downloadData->entry->setSelected(false);
                emit selectionChanged();
                delete _downloadData;
                _downloadData = nullptr;
            }
            QGCLogEntry* entry = _getEntryByLogID(id);
            if (!entry) {
                qWarning() << "No entry found with id" << id;
                //-- no entry with correct id found, ignoring
                return;
            }
            _downloadData = new LogDownloadData(entry);
            _downloadData->elapsed.start();
        }
        _downloadData->written += count;
        _downloadData->rate_bytes += count;
        _updateDataRate();
        return;
    }
    //-- Everything below won't be executed if we are transfering logs
    //----------------------------------------------------------------

    //-- APM "Fix"
    id -= _apmOneBased;
    if(_downloadData->ID != id) {
        qWarning() << "Received log data for wrong log";
        return;
    }

    if ((ofs % MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN) != 0) {
        qWarning() << "Ignored misaligned incoming packet @" << ofs;
        return;
    }

    bool result = false;
    uint32_t timeout_time = kTimeOutMilliseconds;
    if(ofs <= _downloadData->entry->size()) {
        const uint32_t chunk = ofs / kChunkSize;
        if (chunk != _downloadData->current_chunk) {
            qWarning() << "Ignored packet for out of order chunk actual:expected" << chunk << _downloadData->current_chunk;
            return;
        }
        const uint16_t bin = (ofs - chunk*kChunkSize) / MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN;
        if (bin >= _downloadData->chunk_table.size()) {
            qWarning() << "Out of range bin received";
        } else
            _downloadData->chunk_table.setBit(bin);
        if (_downloadData->file.pos() != ofs) {
            // Seek to correct position
            if (!_downloadData->file.seek(ofs)) {
                qWarning() << "Error while seeking log file offset";
                return;
            }
        }

        //-- Write chunk to file
        if(_downloadData->file.write((const char*)data, count)) {
            _downloadData->written += count;
            _downloadData->rate_bytes += count;
            _updateDataRate();
            result = true;
            //-- reset retries
            _retries = 0;
            //-- Reset timer
            _timer.start(timeout_time);
            //-- Do we have it all?
            if(_logComplete()) {
                _downloadData->entry->setStatus(tr("Downloaded"));
                //-- Check for more
                _receivedAllData();
            } else if (_chunkComplete()) {
                _downloadData->advanceChunk();
                _requestLogData(_downloadData->ID,
                                _downloadData->current_chunk*kChunkSize,
                                _downloadData->chunk_table.size()*MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN);
            } else if (bin < _downloadData->chunk_table.size() - 1 && _downloadData->chunk_table.at(bin+1)) {
                // Likely to be grabbing fragments and got to the end of a gap
                _findMissingData();
            }
        } else {
            qWarning() << "Error while writing log file chunk";
        }
    } else {
        qWarning() << "Received log offset greater than expected";
    }
    if(!result) {
        _downloadData->entry->setStatus(tr("Error"));
    }
}

//----------------------------------------------------------------------------------------
QGCLogEntry*
LogDownloadController::_getEntryByLogID(uint16_t id)
{
    int num_logs = _logEntriesModel.count();
    for(int i = 0; i < num_logs; i++) { 
        QGCLogEntry* entry = _logEntriesModel[i];
        if(entry) {
            if (entry->id() == id) {
                return entry;
            }
        }
    }
    return nullptr;
}

//----------------------------------------------------------------------------------------
bool
LogDownloadController::_chunkComplete() const
{
    return _downloadData->chunkEquals(true);
}

//----------------------------------------------------------------------------------------
bool
LogDownloadController::_logComplete() const
{
    return _chunkComplete() && (_downloadData->current_chunk+1) == _downloadData->numChunks();
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_receivedAllData()
{
    _timer.stop();
    //-- Anything queued up for download?
    if(_prepareLogDownload()) {
        //-- Request Log
        _requestLogData(_downloadData->ID, 0, _downloadData->chunk_table.size()*MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN);
        _timer.start(kTimeOutMilliseconds);
    } else {
        _resetSelection();
        _setDownloading(false);
        _setTransfering(false);
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_findMissingData()
{
    if (_logComplete()) {
         _receivedAllData();
         return;
    } else if (_chunkComplete()) {
        _downloadData->advanceChunk();
    }

    _retries++;
#if 0
    // Trying the change to infinite log download. This way if retries hit 100% failure the data rate will
    // slowly fall to 0 and the user can Cancel. This should work better on really crappy links.
    if(_retries > 5) {
        _downloadData->entry->setStatus(tr("Timed Out"));
        //-- Give up
        qWarning() << "Too many errors retreiving log data. Giving up.";
        _receivedAllData();
        return;
    }
#endif

    _updateDataRate();

    uint16_t start = 0, end = 0;
    const int size = _downloadData->chunk_table.size();
    for (; start < size; start++) {
        if (!_downloadData->chunk_table.testBit(start)) {
            break;
        }
    }

    for (end = start; end < size; end++) {
        if (_downloadData->chunk_table.testBit(end)) {
            break;
        }
    }

    const uint32_t pos = _downloadData->current_chunk*kChunkSize + start*MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN,
                   len = (end - start)*MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN;
    _requestLogData(_downloadData->ID, pos, len, _retries);
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_requestLogData(uint16_t id, uint32_t offset, uint32_t count, int retryCount)
{
    if (_vehicle) {
        WeakLinkInterfacePtr weakLink = _vehicle->vehicleLinkManager()->primaryLink();
        if (!weakLink.expired()) {
            SharedLinkInterfacePtr sharedLink = weakLink.lock();

            //-- APM "Fix"
            id += _apmOneBased;
            qCDebug(LogDownloadLog) << "Request log data (id:" << id << "offset:" << offset << "size:" << count << "retryCount" << retryCount << ")";
            mavlink_message_t msg;
            mavlink_msg_log_request_data_pack_chan(
                        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                        sharedLink->mavlinkChannel(),
                        &msg,
                        _vehicle->id(), _vehicle->defaultComponentId(),
                        id, offset, count);
            _vehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), msg);
        }
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::transfer(void)
{
    startLogTransfer();
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::startLogTransfer(void)
{
    if(_downloadingLogs) {
        //-- make sure we are not in the progress of downloading logs
        return;
    }

    //-- mark that we are in transfering mode
    _setListing(false); //-- in case we are still waiting for log entries to arrive
    _setTransfering(true);

    //-- stop the timer since we don't need it for the log transfer
    _timer.stop();

    //-- check if we have any selected entries, in that case we only want to download those
    bool selected_entries = false;
    int num_logs = _logEntriesModel.count();
    for(int i = 0; i < num_logs; i++) {
        QGCLogEntry* entry = _logEntriesModel[i];
        if(entry) {
            if(entry->selected()) {
                selected_entries = true;
                break;
            }
        }
    }

    if(selected_entries) {
        //-- if we have selected entries, mark them as waiting...
        _setSelectedStatus("Waiting");

        //-- ... and get the first one for transfering
        QGCLogEntry* next_entry = _getNextSelected();

        //-- create a new download data to track the download progress
        delete _downloadData;
        _downloadData = new LogDownloadData(next_entry);
        _downloadData->elapsed.start();

        //-- request the transfer of the first selected log file
        _sendLogTransferRequest(next_entry->id());
    }
    else {
        //-- no specific log files requested, transfer all available log files,
        _completeTransfer = true;

        //-- this will always first refresh the list of log files from the
        //-- pixhawk so we can clear the list
        if(_logEntriesModel.count()){
            _logEntriesModel.clear();
        }

        //-- send the request for a complete transfer and wait for log entries
        _sendLogTransferRequest(UINT16_MAX);
        _setListing(true);
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_setSelectedStatus(QString status)
{
    int num_logs = _logEntriesModel.count();
    for(int i = 0; i < num_logs; i++) {
        QGCLogEntry* entry = _logEntriesModel[i];
        if(entry) {
            if(entry->selected()) {
                entry->setStatus(status);
            }
        }
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_sendLogTransferRequest(int id)
{
    if (id >= UINT16_MAX){
        id = UINT16_MAX; // mavlink field for id is only uint_16t
        qCDebug(LogDownloadLog) << "Request log transfer of all available logs";
    } else {
        qCDebug(LogDownloadLog) << "Request log transfer of log with id: " << id;
    }

    SharedLinkInterfacePtr sharedLink = _getLink();
    if(!sharedLink) {
        return;
    }

    mavlink_message_t msg;
    mavlink_msg_log_trs_pack_chan(
        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
        sharedLink->mavlinkChannel(),
        &msg,
        254,
        0,
        this->_since_start_timer.elapsed()*1000,
        id);
    _sendMavlinkMessage(msg, sharedLink);
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_sendLogTransferCancel(void)
{
    qCDebug(LogDownloadLog) << "Cancelling log transfer";
    SharedLinkInterfacePtr sharedLink = _getLink();
    if(!sharedLink) {
        return;
    }

    mavlink_message_t msg;
    mavlink_msg_log_cnc_pack_chan(
        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
        sharedLink->mavlinkChannel(),
        &msg,
        254,
        0,
        this->_since_start_timer.elapsed()*1000);
    _sendMavlinkMessage(msg, sharedLink);
}

//----------------------------------------------------------------------------------------
SharedLinkInterfacePtr
LogDownloadController::_getLink()
{
    if (_vehicle) {
        WeakLinkInterfacePtr weakLink = _vehicle->vehicleLinkManager()->primaryLink();
        if (!weakLink.expired()) {
            return weakLink.lock();
        }
    }
    qCDebug(LogDownloadLog) << "Tried to send mavlink message but failed";
    return nullptr;
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_sendMavlinkMessage(mavlink_message_t& msg, SharedLinkInterfacePtr link_interface)
{
    _vehicle->sendMessageOnLinkThreadSafe(link_interface.get(), msg);
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::refresh(void)
{
    _logEntriesModel.clear();
    //-- Get first 50 entries
    _requestLogList(0, 49);
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_requestLogList(uint32_t start, uint32_t end)
{
    if(_vehicle && _uas) {
        qCDebug(LogDownloadLog) << "Request log entry list (" << start << "through" << end << ")";
        _setListing(true);
        WeakLinkInterfacePtr weakLink = _vehicle->vehicleLinkManager()->primaryLink();
        if (!weakLink.expired()) {
            SharedLinkInterfacePtr sharedLink = weakLink.lock();

            mavlink_message_t msg;
            mavlink_msg_log_request_list_pack_chan(
                        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                        sharedLink->mavlinkChannel(),
                        &msg,
                        _vehicle->id(),
                        _vehicle->defaultComponentId(),
                        start,
                        end);
            _vehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), msg);
        }
        //-- Wait 5 seconds before bitching about not getting anything
        _timer.start(5000);
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::download(QString path)
{
    QString dir = path;
    if (dir.isEmpty()) {
        dir = qgcApp()->toolbox()->settingsManager()->appSettings()->logSavePath();
    }
    downloadToDirectory(dir);
}

//----------------------------------------------------------------------------------------
void LogDownloadController::downloadToDirectory(const QString& dir)
{
    //-- Stop listing just in case
    _receivedAllEntries();
    //-- Reset downloads, again just in case
    delete _downloadData;
    _downloadData = nullptr;

    _downloadPath = dir;
    if(!_downloadPath.isEmpty()) {
        if(!_downloadPath.endsWith(QDir::separator()))
            _downloadPath += QDir::separator();
        //-- Iterate selected entries and shown them as waiting
        _setSelectedStatus(tr("Waiting"));
        //-- Start download process
        _setDownloading(true);
        _receivedAllData();
    }
}

//----------------------------------------------------------------------------------------
QGCLogEntry*
LogDownloadController::_getNextSelected()
{
    //-- Iterate entries and look for a selected file
    int num_logs = _logEntriesModel.count();
    for(int i = 0; i < num_logs; i++) {
        QGCLogEntry* entry = _logEntriesModel[i];
        if(entry) {
            if(entry->selected()) {
               return entry;
            }
        }
    }
    return nullptr;
}

//----------------------------------------------------------------------------------------
bool
LogDownloadController::_prepareLogDownload()
{
    delete _downloadData;
    _downloadData = nullptr;

    QGCLogEntry* entry = _getNextSelected();
    if(!entry) {
        return false;
    }
    //-- Deselect file
    entry->setSelected(false);
    emit selectionChanged();
    bool result = false;
    QString ftime;
    if(entry->time().date().year() < 2010) {
        ftime = tr("UnknownDate");
    } else {
        ftime = entry->time().toString(QStringLiteral("yyyy-M-d-hh-mm-ss"));
    }
    _downloadData = new LogDownloadData(entry);
    _downloadData->filename = QString("log_") + QString::number(entry->id()) + "_" + ftime;
    if (_vehicle->firmwareType() == MAV_AUTOPILOT_PX4) {
        QString loggerParam = QStringLiteral("SYS_LOGGER");
        if (_vehicle->parameterManager()->parameterExists(FactSystem::defaultComponentId, loggerParam) &&
                _vehicle->parameterManager()->getParameter(FactSystem::defaultComponentId, loggerParam)->rawValue().toInt() == 0) {
            _downloadData->filename += ".px4log";
        } else {
            _downloadData->filename += ".ulg";
        }
    } else {
        _downloadData->filename += ".bin";
    }
    _downloadData->file.setFileName(_downloadPath + _downloadData->filename);
    //-- Append a number to the end if the filename already exists
    if (_downloadData->file.exists()){
        uint num_dups = 0;
        QStringList filename_spl = _downloadData->filename.split('.');
        do {
            num_dups +=1;
            _downloadData->file.setFileName(filename_spl[0] + '_' + QString::number(num_dups) + '.' + filename_spl[1]);
        } while( _downloadData->file.exists());
    }
    //-- Create file
    if (!_downloadData->file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create log file:" <<  _downloadData->filename;
    } else {
        //-- Preallocate file
        if(!_downloadData->file.resize(entry->size())) {
            qWarning() << "Failed to allocate space for log file:" <<  _downloadData->filename;
        } else {
            _downloadData->current_chunk = 0;
            _downloadData->chunk_table = QBitArray(_downloadData->chunkBins(), false);
            _downloadData->elapsed.start();
            result = true;
        }
    }
    if(!result) {
        if (_downloadData->file.exists()) {
            _downloadData->file.remove();
        }
        _downloadData->entry->setStatus(tr("Error"));
        delete _downloadData;
        _downloadData = nullptr;
    }
    return result;
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_setDownloading(bool active)
{
    if (_downloadingLogs != active) {
        _downloadingLogs = active;
        _vehicle->vehicleLinkManager()->setCommunicationLostEnabled(!active);
        emit downloadingLogsChanged();
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_setTransfering(bool active)
{
    if (_transferingLogs != active) {
        _transferingLogs = active;
        _vehicle->vehicleLinkManager()->setCommunicationLostEnabled(!active);
        emit transferingLogsChanged();
    }
    //-- if _completeTransfer was true and we are stopping, reset it
    _completeTransfer = active && _completeTransfer;
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::_setListing(bool active)
{
    if (_requestingLogEntries != active) {
        _requestingLogEntries = active;
        _vehicle->vehicleLinkManager()->setCommunicationLostEnabled(!active);
        emit requestingListChanged();
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::eraseAll(void)
{
    if(_vehicle && _uas) {
        WeakLinkInterfacePtr weakLink = _vehicle->vehicleLinkManager()->primaryLink();
        if (!weakLink.expired()) {
            SharedLinkInterfacePtr sharedLink = weakLink.lock();

            mavlink_message_t msg;
            mavlink_msg_log_erase_pack_chan(
                        qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                        qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                        sharedLink->mavlinkChannel(),
                        &msg,
                        qgcApp()->toolbox()->multiVehicleManager()->activeVehicle()->id(), qgcApp()->toolbox()->multiVehicleManager()->activeVehicle()->defaultComponentId());
            _vehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), msg);
        }
        refresh();
    }
}

//----------------------------------------------------------------------------------------
void
LogDownloadController::cancel(void)
{
    qCDebug(LogDownloadLog) << _uas << _downloadData << _transferingLogs;
    if(_uas){
        _receivedAllEntries();
    }
    if(_downloadData) {
        _downloadData->entry->setStatus(tr("Canceled"));
        if (!_transferingLogs){
            if(_downloadData->file.exists()) {
                _downloadData->file.remove();
            }
        }
        delete _downloadData;
        _downloadData = 0;
    }
    if(_transferingLogs) {
        _sendLogTransferCancel();
    }
    _resetSelection(true);
    _setDownloading(false);
    _setTransfering(false);
}

//-----------------------------------------------------------------------------
QGCLogModel::QGCLogModel(QObject* parent)
    : QAbstractListModel(parent)
{

}

//-----------------------------------------------------------------------------
QGCLogEntry*
QGCLogModel::get(int index)
{
    if (index < 0 || index >= _logEntries.count()) {
        return nullptr;
    }
    return _logEntries[index];
}

//-----------------------------------------------------------------------------
int
QGCLogModel::count() const
{
    return _logEntries.count();
}

//-----------------------------------------------------------------------------
void
QGCLogModel::append(QGCLogEntry* object)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
    _logEntries.append(object);
    endInsertRows();
    emit countChanged();
}

//-----------------------------------------------------------------------------
void
QGCLogModel::clear(void)
{
    if(!_logEntries.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, _logEntries.count());
        while (_logEntries.count()) {
            QGCLogEntry* entry = _logEntries.last();
            if(entry) entry->deleteLater();
            _logEntries.removeLast();
        }
        endRemoveRows();
        emit countChanged();
    }
}

//-----------------------------------------------------------------------------
QGCLogEntry*
QGCLogModel::operator[](int index)
{
    return get(index);
}

//-----------------------------------------------------------------------------
int
QGCLogModel::rowCount(const QModelIndex& /*parent*/) const
{
    return _logEntries.count();
}

//-----------------------------------------------------------------------------
QVariant
QGCLogModel::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() >= _logEntries.count())
        return QVariant();
    if (role == ObjectRole)
        return QVariant::fromValue(_logEntries[index.row()]);
    return QVariant();
}

//-----------------------------------------------------------------------------
QHash<int, QByteArray>
QGCLogModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[ObjectRole] = "logEntry";
    return roles;
}
