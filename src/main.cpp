#include <Arduino.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeMonoBoldOblique18pt7b.h>
#include <Fonts/FreeMonoBoldOblique24pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeMonoOblique12pt7b.h>
#include <Fonts/FreeMonoOblique18pt7b.h>
#include <Fonts/FreeMonoOblique24pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono18pt7b.h>
#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Fonts/FreeSansBoldOblique18pt7b.h>
#include <Fonts/FreeSansBoldOblique24pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeSansOblique12pt7b.h>
#include <Fonts/FreeSansOblique18pt7b.h>
#include <Fonts/FreeSansOblique24pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSerifBold24pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>
#include <Fonts/FreeSerifBoldItalic18pt7b.h>
#include <Fonts/FreeSerifBoldItalic24pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeSerifItalic12pt7b.h>
#include <Fonts/FreeSerifItalic18pt7b.h>
#include <Fonts/FreeSerifItalic24pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>

#include <GxEPD2_BW.h>
#include <tinyxml2.h>
#include <LittleFS.h>
#include <JPEGDEC.h>

#ifdef INTELSHORT
#undef INTELSHORT
#endif
#ifdef INTELLONG
#undef INTELLONG
#endif
#ifdef MOTOSHORT
#undef MOTOSHORT
#endif
#ifdef MOTOLONG
#undef MOTOLONG
#endif

#include <PNGdec.h>
#include <SPI.h>
#include <unzipLIB.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Preferences.h>
#include <new>
#include <cstring>
#include <vector>

#define USE_HSPI_FOR_EPD

#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_750_GDEY075T7

#define GxEPD2_BW_IS_GxEPD2_BW true
#define GxEPD2_3C_IS_GxEPD2_3C true
#define GxEPD2_7C_IS_GxEPD2_7C true
#define GxEPD2_1248_IS_GxEPD2_1248 true
#define IS_GxEPD(c, x) (c##x)
#define IS_GxEPD2_BW(x) IS_GxEPD(GxEPD2_BW_IS_, x)
#define IS_GxEPD2_3C(x) IS_GxEPD(GxEPD2_3C_IS_, x)
#define IS_GxEPD2_7C(x) IS_GxEPD(GxEPD2_7C_IS_, x)

#if defined(ESP32)
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#if IS_GxEPD2_BW(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#elif IS_GxEPD2_3C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#elif IS_GxEPD2_7C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2))
#endif
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/15, /*DC=*/27, /*RST=*/26, /*BUSY=*/25));
#endif

#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
SPIClass hspi(HSPI);
#endif

namespace
{
constexpr const char* kApSsid = "ebook-reader";
constexpr const char* kApPassword = "ebook1234";
constexpr uint16_t kWebPort = 80;
constexpr uint16_t kMaxBooks = 64;
constexpr size_t kReaderMaxExtractBytes = 120000;
constexpr size_t kReaderMaxBookTextChars = 1200000;
constexpr size_t kReaderMaxCoverBytes = 300000;
constexpr const char* kReaderCoverTempPath = "/reader_cover.tmp";

WebServer server(kWebPort);
bool wifiServerRunning = false;
bool shutdownRequested = false;
bool uploadSucceeded = false;

enum class ReaderSerialMode : uint8_t
{
	MainMenu,
	DeleteMenu,
	SettingsMenu,
	ReaderStartMenu,
	ReaderJumpPageInput,
	ReaderMenu,
};

enum class ReaderFontFamily : uint8_t
{
	SansSerif,
	Serif,
	Mono,
};

enum class ReaderFontStyle : uint8_t
{
	Normal = 0,
	Bold = 1,
	Oblique = 2,
	BoldOblique = 3,
};

enum class ReaderLineSpacing : uint8_t
{
	Compact = 0,
	Normal = 1,
	Loose = 2,
	VeryLoose = 3,
};

enum class ReaderTextAlign : uint8_t
{
	Left = 0,
	Center = 1,
	Right = 2,
};

ReaderSerialMode serialMode = ReaderSerialMode::MainMenu;
String serialLineBuffer;
enum class SerialNavInput : uint8_t
{
	None,
	Up,
	Down,
	Left,
	Right,
};

uint16_t mainMenuCursor = 0;
uint16_t deleteMenuCursor = 0;
uint16_t settingsMenuCursor = 0;
uint16_t readerStartMenuCursor = 0;

String bookNames[kMaxBooks];
uint16_t bookCount = 0;
File uploadFile;
String uploadPath;
String readerBookPath;
String readerBookTitle;
std::vector<String> readerPages;
uint16_t readerPageIndex = 0;
std::vector<String> readerChapterPaths;
std::vector<String> readerChapterTitles;
uint16_t readerChapterIndex = 0;
String readerCoverPath;
bool readerHasCover = false;
bool readerShowingCover = false;
size_t readerParsePos = 0;
String readerParseCurrentLine;
String readerParseCurrentPage;
String readerParseWord;
ReaderFontStyle readerParseCurrentStyle = ReaderFontStyle::Normal;
ReaderTextAlign readerParseCurrentAlign = ReaderTextAlign::Left;
uint16_t readerParseLineCount = 0;
bool readerChapterParseComplete = true;
bool readerAwaitingStyleCode = false;
bool readerAwaitingAlignCode = false;
String readerParsedTextBuffer;
String readerMarkupTextSegment;
String readerMarkupRawCarry;
bool readerChapterInputEnded = true;
bool readerChapterFileOpen = false;
UNZIP* epubZip = nullptr;
File epubFsFile;

ReaderFontFamily readerFontFamily = ReaderFontFamily::SansSerif;
uint8_t readerFontSizePt = 12;
ReaderLineSpacing readerLineSpacing = ReaderLineSpacing::Normal;
Preferences readerSettingsStore;
constexpr const char* kSettingsNamespace = "reader";
constexpr const char* kIndexCacheNamespace = "reader_idx";
constexpr uint8_t kIndexCacheSlots = 6;
constexpr const char* kBookmarksNamespace = "reader_bm";
constexpr size_t kBookmarkPathBytes = 96;
constexpr uint32_t kBookmarkPersistIntervalMs = 5000;
constexpr const char* kKeyFontFamily = "font_family";
constexpr const char* kKeyFontSize = "font_size";
constexpr const char* kKeyLineSpacing = "line_spacing";
constexpr char kStyleMarker = 0x1D;
constexpr char kAlignMarker = 0x1E;
std::vector<String> readerCssAlignCenterClasses;
std::vector<String> readerCssAlignRightClasses;

struct ReaderBookmark
{
	String bookPath;
	uint16_t chapterIndex = 0;
	uint16_t pageIndex = 0;
	bool showingCover = false;
};

struct ReaderNavEntry
{
	String href;
	String title;
};

ReaderBookmark readerBookmarks[kMaxBooks];
uint16_t readerBookmarkCount = 0;
bool readerBookmarksDirty = false;
uint32_t readerBookmarksLastPersistMs = 0;
uint16_t readerResumeChapterIndex = 0;
uint16_t readerResumePageIndex = 0;
bool readerResumeShowingCover = false;
uint16_t readerCoverReturnPageIndex = 0;
uint16_t readerJumpTargetChapterIndex = 0;
uint16_t readerChapterPageCounts[kMaxBooks] = {0};
uint32_t readerTotalPagesLoaded = 0;

struct ReaderMarkupStreamState
{
	bool inTag = false;
	bool inEntity = false;
	char tagName[16] = {0};
	uint8_t tagLen = 0;
	char tagAttrs[96] = {0};
	uint8_t attrsLen = 0;
	bool collectingTagName = false;
	bool sawTagNameChar = false;
	String entityBuffer;
	uint8_t boldDepth = 0;
	uint8_t obliqueDepth = 0;
	uint8_t headerDepth = 0;
	uint8_t attrBoldDepth = 0;
	uint8_t attrObliqueDepth = 0;
	ReaderFontStyle activeStyle = ReaderFontStyle::Normal;
	ReaderTextAlign activeAlign = ReaderTextAlign::Left;
};

ReaderMarkupStreamState readerMarkupState;

struct ReaderCoverDrawContext
{
	int16_t dstX = 0;
	int16_t dstY = 0;
	uint16_t dstW = 0;
	uint16_t dstH = 0;
	uint16_t srcW = 0;
	uint16_t srcH = 0;
	int16_t xOffset = 0;
	int16_t yOffset = 0;
	uint8_t scaleDiv = 1;
	PNG* pngDecoder = nullptr;
	uint16_t* pngLineBuffer = nullptr;
	size_t pngLineCapacity = 0;
};

static void* openLittleFSFileForPng(const char* filename, int32_t* fileSize)
{
	File* file = new (std::nothrow) File(LittleFS.open(filename, FILE_READ));
	if (!file || !(*file))
	{
		delete file;
		return nullptr;
	}

	if (fileSize)
	{
		*fileSize = static_cast<int32_t>(file->size());
	}
	return file;
}

static void closeLittleFSFileForPng(void* handle)
{
	if (!handle)
	{
		return;
	}

	File* file = reinterpret_cast<File*>(handle);
	file->close();
	delete file;
}

static int32_t readLittleFSFileForPng(PNGFILE* pFile, uint8_t* buffer, int32_t length)
{
	if (!pFile || !pFile->fHandle)
	{
		return -1;
	}

	File* file = reinterpret_cast<File*>(pFile->fHandle);
	return static_cast<int32_t>(file->read(buffer, static_cast<size_t>(length)));
}

static int32_t seekLittleFSFileForPng(PNGFILE* pFile, int32_t position)
{
	if (!pFile || !pFile->fHandle)
	{
		return -1;
	}

	File* file = reinterpret_cast<File*>(pFile->fHandle);
	if (!file->seek(static_cast<uint32_t>(position), SeekSet))
	{
		return -1;
	}

	return position;
}

static void* openLittleFSFileForJpeg(const char* filename, int32_t* fileSize)
{
	File* file = new (std::nothrow) File(LittleFS.open(filename, FILE_READ));
	if (!file || !(*file))
	{
		delete file;
		return nullptr;
	}

	if (fileSize)
	{
		*fileSize = static_cast<int32_t>(file->size());
	}
	return file;
}

static void closeLittleFSFileForJpeg(void* handle)
{
	if (!handle)
	{
		return;
	}

	File* file = reinterpret_cast<File*>(handle);
	file->close();
	delete file;
}

static int32_t readLittleFSFileForJpeg(JPEGFILE* pFile, uint8_t* buffer, int32_t length)
{
	if (!pFile || !pFile->fHandle)
	{
		return 0;
	}

	File* file = reinterpret_cast<File*>(pFile->fHandle);
	return static_cast<int32_t>(file->read(buffer, static_cast<size_t>(length)));
}

static int32_t seekLittleFSFileForJpeg(JPEGFILE* pFile, int32_t position)
{
	if (!pFile || !pFile->fHandle)
	{
		return -1;
	}

	File* file = reinterpret_cast<File*>(pFile->fHandle);
	if (!file->seek(static_cast<uint32_t>(position), SeekSet))
	{
		return -1;
	}

	return position;
}

bool extractNamedZipEntryToFile(UNZIP& zip, const char* entryName, const char* outputPath, size_t maxBytes = kReaderMaxCoverBytes)
{
	if (!entryName || !outputPath)
	{
		return false;
	}

	bool located = (zip.locateFile(entryName) == UNZ_OK);
	size_t expectedSize = 0;
	if (located)
	{
		unz_file_info fileInfo;
		char fileName[UNZ_MAXFILENAMEINZIP];
		if (zip.getFileInfo(&fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0) == UNZ_OK)
		{
			expectedSize = static_cast<size_t>(fileInfo.uncompressed_size);
		}
	}

	if (!located)
	{
		String alternate = String(entryName);
		if (alternate.startsWith("/"))
		{
			alternate = alternate.substring(1);
		}
		else
		{
			alternate = "/" + alternate;
		}

		located = (zip.locateFile(alternate.c_str()) == UNZ_OK);
		if (located)
		{
			unz_file_info fileInfo;
			char fileName[UNZ_MAXFILENAMEINZIP];
			if (zip.getFileInfo(&fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0) == UNZ_OK)
			{
				expectedSize = static_cast<size_t>(fileInfo.uncompressed_size);
			}
		}
	}

	if (!located)
	{
		String wantedName = String(entryName);
		const int wantedSlash = wantedName.lastIndexOf('/');
		String wantedBase = wantedSlash >= 0 ? wantedName.substring(wantedSlash + 1) : wantedName;
		wantedBase.toLowerCase();

		if (zip.gotoFirstFile() == UNZ_OK)
		{
			int rc = UNZ_OK;
			while (rc == UNZ_OK)
			{
				char fileName[UNZ_MAXFILENAMEINZIP];
				unz_file_info fileInfo;
				rc = zip.getFileInfo(&fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0);
				if (rc == UNZ_OK)
				{
					String candidate = String(fileName);
					String candidateLower = candidate;
					candidateLower.toLowerCase();

					if (candidate == String(entryName))
					{
						located = true;
						expectedSize = static_cast<size_t>(fileInfo.uncompressed_size);
						break;
					}

					const int slash = candidateLower.lastIndexOf('/');
					const String candidateBase = slash >= 0 ? candidateLower.substring(slash + 1) : candidateLower;
					if (candidateBase == wantedBase)
					{
						located = true;
						expectedSize = static_cast<size_t>(fileInfo.uncompressed_size);
						break;
					}
				}

				rc = zip.gotoNextFile();
				yield();
			}
		}
	}

	if (!located)
	{
		Serial.print("[COVER] locateFile failed for: ");
		Serial.println(entryName);
		return false;
	}

	if (expectedSize > maxBytes)
	{
		Serial.println("[COVER] cover exceeds configured max size");
		return false;
	}

	LittleFS.remove(outputPath);
	File outputFile = LittleFS.open(outputPath, FILE_WRITE);
	if (!outputFile)
	{
		Serial.print("[COVER] failed to open temp file: ");
		Serial.println(outputPath);
		return false;
	}

	if (zip.openCurrentFile() != UNZ_OK)
	{
		Serial.println("[COVER] openCurrentFile failed");
		outputFile.close();
		LittleFS.remove(outputPath);
		return false;
	}

	uint8_t chunk[512];
	int32_t bytesRead = 0;
	size_t totalRead = 0;
	while ((bytesRead = zip.readCurrentFile(chunk, sizeof(chunk))) > 0)
	{
		const size_t wanted = static_cast<size_t>(bytesRead);
		if (totalRead + wanted > maxBytes)
		{
			Serial.println("[COVER] cover exceeds max buffer size");
			zip.closeCurrentFile();
			outputFile.close();
			LittleFS.remove(outputPath);
			return false;
		}

		const size_t written = outputFile.write(chunk, wanted);
		if (written != wanted)
		{
			Serial.println("[COVER] temp file write failed");
			zip.closeCurrentFile();
			outputFile.close();
			LittleFS.remove(outputPath);
			return false;
		}

		totalRead += wanted;
		if ((totalRead & 0x0FFF) == 0)
		{
			yield();
		}
	}

	zip.closeCurrentFile();
	outputFile.flush();
	outputFile.close();
	return bytesRead >= 0;
}

struct ReaderBookmarkStorageRecord
{
	char path[kBookmarkPathBytes];
	uint16_t chapterIndex;
	uint16_t pageIndex;
	uint8_t showingCover;
	uint8_t reserved;
};

bool isSupportedFontSize(uint8_t sizePt)
{
	return sizePt == 9 || sizePt == 12 || sizePt == 18 || sizePt == 24;
}

uint8_t clampFontSize(uint8_t sizePt)
{
	if (isSupportedFontSize(sizePt))
	{
		return sizePt;
	}
	if (sizePt <= 10)
	{
		return 9;
	}
	if (sizePt <= 15)
	{
		return 12;
	}
	if (sizePt <= 21)
	{
		return 18;
	}
	return 24;
}

bool saveReaderSettings()
{
	if (!readerSettingsStore.begin(kSettingsNamespace, false))
	{
		Serial.println("Failed to open settings storage for write");
		return false;
	}

	const bool familyOk = readerSettingsStore.putUChar(kKeyFontFamily, static_cast<uint8_t>(readerFontFamily)) == sizeof(uint8_t);
	const bool sizeOk = readerSettingsStore.putUChar(kKeyFontSize, readerFontSizePt) == sizeof(uint8_t);
	const bool spacingOk = readerSettingsStore.putUChar(kKeyLineSpacing, static_cast<uint8_t>(readerLineSpacing)) == sizeof(uint8_t);
	readerSettingsStore.end();

	if (!familyOk || !sizeOk || !spacingOk)
	{
		Serial.println("Failed to persist reader settings");
		return false;
	}

	return true;
}

bool flushReaderBookmarksToStorage(bool forceWrite)
{
	if (!forceWrite)
	{
		if (!readerBookmarksDirty)
		{
			return true;
		}

		const uint32_t now = millis();
		if (readerBookmarksLastPersistMs != 0 && (now - readerBookmarksLastPersistMs) < kBookmarkPersistIntervalMs)
		{
			return true;
		}
	}

	Preferences bookmarksStore;
	if (!bookmarksStore.begin(kBookmarksNamespace, false))
	{
		Serial.println("Failed to open bookmark storage for write");
		return false;
	}

	const uint16_t count = readerBookmarkCount <= kMaxBooks ? readerBookmarkCount : kMaxBooks;
	const size_t bytes = static_cast<size_t>(count) * sizeof(ReaderBookmarkStorageRecord);
	ReaderBookmarkStorageRecord* records = nullptr;
	if (bytes > 0)
	{
		records = static_cast<ReaderBookmarkStorageRecord*>(malloc(bytes));
		if (!records)
		{
			bookmarksStore.end();
			Serial.println("Failed to allocate bookmark storage buffer");
			return false;
		}

		for (uint16_t i = 0; i < count; ++i)
		{
			ReaderBookmarkStorageRecord& rec = records[i];
			memset(&rec, 0, sizeof(rec));
			const String& path = readerBookmarks[i].bookPath;
			if (!path.isEmpty())
			{
				strncpy(rec.path, path.c_str(), kBookmarkPathBytes - 1);
				rec.path[kBookmarkPathBytes - 1] = '\0';
			}
			rec.chapterIndex = readerBookmarks[i].chapterIndex;
			rec.pageIndex = readerBookmarks[i].pageIndex;
			rec.showingCover = readerBookmarks[i].showingCover ? 1 : 0;
		}
	}

	const bool countOk = bookmarksStore.putUShort("count", count) == sizeof(uint16_t);
	bool dataOk = true;
	if (bytes > 0)
	{
		dataOk = bookmarksStore.putBytes("data", records, bytes) == bytes;
	}
	else
	{
		bookmarksStore.remove("data");
	}

	bookmarksStore.end();
	if (records)
	{
		free(records);
	}

	if (countOk && dataOk)
	{
		readerBookmarksDirty = false;
		readerBookmarksLastPersistMs = millis();
		return true;
	}

	Serial.println("Failed to persist bookmarks");
	return false;
}

void loadReaderBookmarks()
{
	readerBookmarkCount = 0;
	readerBookmarksDirty = false;

	Preferences bookmarksStore;
	if (!bookmarksStore.begin(kBookmarksNamespace, true))
	{
		Serial.println("Failed to open bookmark storage for read");
		return;
	}

	uint16_t storedCount = bookmarksStore.getUShort("count", 0);
	if (storedCount > kMaxBooks)
	{
		storedCount = kMaxBooks;
	}

	const size_t storedBytes = bookmarksStore.getBytesLength("data");
	const size_t recSize = sizeof(ReaderBookmarkStorageRecord);
	if (storedCount == 0 || storedBytes < recSize)
	{
		bookmarksStore.end();
		return;
	}

	size_t maxRecordsByBytes = storedBytes / recSize;
	if (maxRecordsByBytes > kMaxBooks)
	{
		maxRecordsByBytes = kMaxBooks;
	}
	if (storedCount > maxRecordsByBytes)
	{
		storedCount = static_cast<uint16_t>(maxRecordsByBytes);
	}

	const size_t bytesToRead = static_cast<size_t>(storedCount) * recSize;
	ReaderBookmarkStorageRecord* records = static_cast<ReaderBookmarkStorageRecord*>(malloc(bytesToRead));
	if (!records)
	{
		bookmarksStore.end();
		Serial.println("Failed to allocate bookmark read buffer");
		return;
	}

	const size_t bytesRead = bookmarksStore.getBytes("data", records, bytesToRead);
	bookmarksStore.end();
	if (bytesRead != bytesToRead)
	{
		free(records);
		Serial.println("Failed to read persisted bookmarks");
		return;
	}

	for (uint16_t i = 0; i < storedCount && readerBookmarkCount < kMaxBooks; ++i)
	{
		const ReaderBookmarkStorageRecord& rec = records[i];
		if (rec.path[0] == '\0')
		{
			continue;
		}

		ReaderBookmark& bm = readerBookmarks[readerBookmarkCount++];
		bm.bookPath = String(rec.path);
		bm.chapterIndex = rec.chapterIndex;
		bm.pageIndex = rec.pageIndex;
		bm.showingCover = rec.showingCover != 0;
	}

	free(records);
}

void loadReaderSettings()
{
	if (!readerSettingsStore.begin(kSettingsNamespace, true))
	{
		Serial.println("Failed to open settings storage for read");
		return;
	}

	const uint8_t storedFamily = readerSettingsStore.getUChar(kKeyFontFamily, static_cast<uint8_t>(ReaderFontFamily::SansSerif));
	const uint8_t storedSize = readerSettingsStore.getUChar(kKeyFontSize, 12);
	const uint8_t storedSpacing = readerSettingsStore.getUChar(kKeyLineSpacing, static_cast<uint8_t>(ReaderLineSpacing::Normal));
	readerSettingsStore.end();

	if (storedFamily <= static_cast<uint8_t>(ReaderFontFamily::Mono))
	{
		readerFontFamily = static_cast<ReaderFontFamily>(storedFamily);
	}
	else
	{
		readerFontFamily = ReaderFontFamily::SansSerif;
	}

	readerFontSizePt = clampFontSize(storedSize);

	if (storedSpacing <= static_cast<uint8_t>(ReaderLineSpacing::VeryLoose))
	{
		readerLineSpacing = static_cast<ReaderLineSpacing>(storedSpacing);
	}
	else
	{
		readerLineSpacing = ReaderLineSpacing::Normal;
	}
}

const GFXfont* fontForSettings(ReaderFontFamily family, uint8_t sizePt, ReaderFontStyle style)
{
	sizePt = clampFontSize(sizePt);
	if (family == ReaderFontFamily::Serif)
	{
		switch (style)
		{
			case ReaderFontStyle::Bold:
				if (sizePt == 9) return &FreeSerifBold9pt7b;
				if (sizePt == 12) return &FreeSerifBold12pt7b;
				if (sizePt == 18) return &FreeSerifBold18pt7b;
				return &FreeSerifBold24pt7b;
			case ReaderFontStyle::Oblique:
				if (sizePt == 9) return &FreeSerifItalic9pt7b;
				if (sizePt == 12) return &FreeSerifItalic12pt7b;
				if (sizePt == 18) return &FreeSerifItalic18pt7b;
				return &FreeSerifItalic24pt7b;
			case ReaderFontStyle::BoldOblique:
				if (sizePt == 9) return &FreeSerifBoldItalic9pt7b;
				if (sizePt == 12) return &FreeSerifBoldItalic12pt7b;
				if (sizePt == 18) return &FreeSerifBoldItalic18pt7b;
				return &FreeSerifBoldItalic24pt7b;
			default:
				if (sizePt == 9) return &FreeSerif9pt7b;
				if (sizePt == 12) return &FreeSerif12pt7b;
				if (sizePt == 18) return &FreeSerif18pt7b;
				return &FreeSerif24pt7b;
		}
	}

	if (family == ReaderFontFamily::Mono)
	{
		switch (style)
		{
			case ReaderFontStyle::Bold:
				if (sizePt == 9) return &FreeMonoBold9pt7b;
				if (sizePt == 12) return &FreeMonoBold12pt7b;
				if (sizePt == 18) return &FreeMonoBold18pt7b;
				return &FreeMonoBold24pt7b;
			case ReaderFontStyle::Oblique:
				if (sizePt == 9) return &FreeMonoOblique9pt7b;
				if (sizePt == 12) return &FreeMonoOblique12pt7b;
				if (sizePt == 18) return &FreeMonoOblique18pt7b;
				return &FreeMonoOblique24pt7b;
			case ReaderFontStyle::BoldOblique:
				if (sizePt == 9) return &FreeMonoBoldOblique9pt7b;
				if (sizePt == 12) return &FreeMonoBoldOblique12pt7b;
				if (sizePt == 18) return &FreeMonoBoldOblique18pt7b;
				return &FreeMonoBoldOblique24pt7b;
			default:
				if (sizePt == 9) return &FreeMono9pt7b;
				if (sizePt == 12) return &FreeMono12pt7b;
				if (sizePt == 18) return &FreeMono18pt7b;
				return &FreeMono24pt7b;
		}
	}

	switch (style)
	{
		case ReaderFontStyle::Bold:
			if (sizePt == 9) return &FreeSansBold9pt7b;
			if (sizePt == 12) return &FreeSansBold12pt7b;
			if (sizePt == 18) return &FreeSansBold18pt7b;
			return &FreeSansBold24pt7b;
		case ReaderFontStyle::Oblique:
			if (sizePt == 9) return &FreeSansOblique9pt7b;
			if (sizePt == 12) return &FreeSansOblique12pt7b;
			if (sizePt == 18) return &FreeSansOblique18pt7b;
			return &FreeSansOblique24pt7b;
		case ReaderFontStyle::BoldOblique:
			if (sizePt == 9) return &FreeSansBoldOblique9pt7b;
			if (sizePt == 12) return &FreeSansBoldOblique12pt7b;
			if (sizePt == 18) return &FreeSansBoldOblique18pt7b;
			return &FreeSansBoldOblique24pt7b;
		default:
			if (sizePt == 9) return &FreeSans9pt7b;
			if (sizePt == 12) return &FreeSans12pt7b;
			if (sizePt == 18) return &FreeSans18pt7b;
			return &FreeSans24pt7b;
	}
}

void setDisplayFont(ReaderFontStyle style)
{
	display.setFont(fontForSettings(readerFontFamily, readerFontSizePt, style));
}

uint16_t uiCharsPerLine(uint16_t pixelWidth)
{
	uint16_t avgWidth = static_cast<uint16_t>((readerFontSizePt * 56U) / 100U);
	if (readerFontFamily == ReaderFontFamily::Mono)
	{
		avgWidth = static_cast<uint16_t>((readerFontSizePt * 62U) / 100U);
	}
	else if (readerFontFamily == ReaderFontFamily::Serif)
	{
		avgWidth = static_cast<uint16_t>((readerFontSizePt * 58U) / 100U);
	}
	const uint16_t safeAvgWidth = avgWidth < 4 ? 4 : avgWidth;
	const uint16_t chars = pixelWidth / safeAvgWidth;
	return chars < 4 ? 4 : chars;
}

uint16_t uiLineStep()
{
	uint16_t step = static_cast<uint16_t>(readerFontSizePt + 5);
	if (step < 12)
	{
		step = 12;
	}
	switch (readerLineSpacing)
	{
		case ReaderLineSpacing::Compact:
			break;
		case ReaderLineSpacing::Normal:
			step = static_cast<uint16_t>((step * 108U + 50U) / 100U);
			break;
		case ReaderLineSpacing::Loose:
			step = static_cast<uint16_t>((step * 120U + 50U) / 100U);
			break;
		case ReaderLineSpacing::VeryLoose:
			step = static_cast<uint16_t>((step * 145U + 50U) / 100U);
			break;
		default:
			break;
	}
	return step < 12 ? 12 : step;
}

int16_t uiScreenHeaderY()
{
	return static_cast<int16_t>(readerFontSizePt + 12);
}

int16_t uiScreenBodyStartY()
{
	return static_cast<int16_t>(uiScreenHeaderY() + static_cast<int16_t>(uiLineStep()) + 4);
}

uint16_t uiReaderBodyStartY()
{
	return static_cast<uint16_t>(readerFontSizePt + 16 + uiLineStep());
}

String fontFamilyLabel()
{
	if (readerFontFamily == ReaderFontFamily::Mono)
	{
		return "Mono";
	}
	if (readerFontFamily == ReaderFontFamily::Serif)
	{
		return "Serif";
	}
	return "Sans serif";
}

String lineSpacingLabel()
{
	switch (readerLineSpacing)
	{
		case ReaderLineSpacing::Compact:
			return "Compact";
		case ReaderLineSpacing::Normal:
			return "Normal";
		case ReaderLineSpacing::Loose:
			return "Loose";
		case ReaderLineSpacing::VeryLoose:
			return "Very loose";
		default:
			return "Normal";
	}
}

void cycleLineSpacing()
{
	if (readerLineSpacing == ReaderLineSpacing::Compact)
	{
		readerLineSpacing = ReaderLineSpacing::Normal;
	}
	else if (readerLineSpacing == ReaderLineSpacing::Normal)
	{
		readerLineSpacing = ReaderLineSpacing::Loose;
	}
	else if (readerLineSpacing == ReaderLineSpacing::Loose)
	{
		readerLineSpacing = ReaderLineSpacing::VeryLoose;
	}
	else
	{
		readerLineSpacing = ReaderLineSpacing::Compact;
	}
}

bool isStyleCodeChar(char code)
{
	return code >= '0' && code <= '3';
}

bool isAlignCodeChar(char code)
{
	return code >= '0' && code <= '2';
}

ReaderFontStyle styleFromCodeChar(char code)
{
	if (!isStyleCodeChar(code))
	{
		return ReaderFontStyle::Normal;
	}
	return static_cast<ReaderFontStyle>(code - '0');
}

ReaderTextAlign alignFromCodeChar(char code)
{
	if (!isAlignCodeChar(code))
	{
		return ReaderTextAlign::Left;
	}
	return static_cast<ReaderTextAlign>(code - '0');
}

void appendStyleMarker(String& encoded, ReaderFontStyle style)
{
	encoded += kStyleMarker;
	encoded += static_cast<char>('0' + static_cast<uint8_t>(style));
}

void appendAlignMarker(String& encoded, ReaderTextAlign align)
{
	encoded += kAlignMarker;
	encoded += static_cast<char>('0' + static_cast<uint8_t>(align));
}

String encodeAlignedLine(const String& encodedLine, ReaderTextAlign align)
{
	if (align == ReaderTextAlign::Left)
	{
		return encodedLine;
	}

	String result;
	result.reserve(encodedLine.length() + 2);
	appendAlignMarker(result, align);
	result += encodedLine;
	return result;
}

void decodeLineAlignment(const String& encoded, ReaderTextAlign& align, String& content)
{
	if (encoded.length() >= 2 && encoded[0] == kAlignMarker && isAlignCodeChar(encoded[1]))
	{
		align = alignFromCodeChar(encoded[1]);
		content = encoded.substring(2);
		return;
	}

	align = ReaderTextAlign::Left;
	content = encoded;
}

ReaderFontStyle trailingStyleForEncodedText(const String& encoded)

{
	ReaderFontStyle style = ReaderFontStyle::Normal;
	for (size_t i = 0; i + 1 < encoded.length(); ++i)
	{
		if (encoded[i] == kStyleMarker && isStyleCodeChar(encoded[i + 1]))
		{
			style = styleFromCodeChar(encoded[i + 1]);
			++i;
		}
	}
	return style;
}

uint16_t measureEncodedTextWidth(const String& encoded)
{
	ReaderFontStyle style = ReaderFontStyle::Normal;
	String chunk;
	chunk.reserve(32);
	uint16_t totalWidth = 0;

	auto flushChunk = [&]()
	{
		if (chunk.isEmpty())
		{
			return;
		}
		setDisplayFont(style);
		int16_t x1, y1;
		uint16_t w, h;
		display.getTextBounds(chunk.c_str(), 0, 0, &x1, &y1, &w, &h);
		totalWidth = static_cast<uint16_t>(totalWidth + w);
		chunk = "";
	};

	for (size_t i = 0; i < encoded.length(); ++i)
	{
		const char ch = encoded[i];
		if (ch == kAlignMarker && (i + 1) < encoded.length() && isAlignCodeChar(encoded[i + 1]))
		{
			++i;
			continue;
		}
		if (ch == kStyleMarker && (i + 1) < encoded.length() && isStyleCodeChar(encoded[i + 1]))
		{
			flushChunk();
			style = styleFromCodeChar(encoded[i + 1]);
			++i;
			continue;
		}
		chunk += ch;
	}
	flushChunk();
	return totalWidth;
}

void drawEncodedTextLine(int16_t x, int16_t y, const String& encoded)
{
	ReaderFontStyle style = ReaderFontStyle::Normal;
	String chunk;
	chunk.reserve(32);
	int16_t cursorX = x;

	auto drawChunk = [&]()
	{
		if (chunk.isEmpty())
		{
			return;
		}
		setDisplayFont(style);
		display.setCursor(cursorX, y);
		display.print(chunk);
		int16_t x1, y1;
		uint16_t w, h;
		display.getTextBounds(chunk.c_str(), 0, 0, &x1, &y1, &w, &h);
		cursorX = static_cast<int16_t>(cursorX + static_cast<int16_t>(w));
		chunk = "";
	};

	for (size_t i = 0; i < encoded.length(); ++i)
	{
		const char ch = encoded[i];
		if (ch == kAlignMarker && (i + 1) < encoded.length() && isAlignCodeChar(encoded[i + 1]))
		{
			++i;
			continue;
		}
		if (ch == kStyleMarker && (i + 1) < encoded.length() && isStyleCodeChar(encoded[i + 1]))
		{
			drawChunk();
			style = styleFromCodeChar(encoded[i + 1]);
			++i;
			continue;
		}
		chunk += ch;
	}
	drawChunk();
}

bool decodeNumericEntityToAscii(const String& entity, String& replacement)
{
	if (!entity.startsWith("#"))
	{
		return false;
	}

	const bool isHex = entity.length() > 2 && (entity[1] == 'x' || entity[1] == 'X');
	const char* digits = entity.c_str() + (isHex ? 2 : 1);
	char* endPtr = nullptr;
	const long codepoint = strtol(digits, &endPtr, isHex ? 16 : 10);
	if (!endPtr || *endPtr != '\0')
	{
		return false;
	}

	if (codepoint >= 32 && codepoint <= 126)
	{
		replacement = String(static_cast<char>(codepoint));
		return true;
	}

	switch (codepoint)
	{
		case 160: replacement = " "; return true;
		case 8211: replacement = "-"; return true;
		case 8212: replacement = "--"; return true;
		case 8216:
		case 8217:
		case 8242: replacement = "'"; return true;
		case 8220:
		case 8221:
		case 8243: replacement = "\""; return true;
		case 8230: replacement = "..."; return true;
		case 8722: replacement = "-"; return true;
		default: break;
	}

	return false;
}

bool decodeEntityToAscii(const String& entity, String& replacement)
{
	if (decodeNumericEntityToAscii(entity, replacement))
	{
		return true;
	}

	if (entity == "amp") { replacement = "&"; return true; }
	if (entity == "lt") { replacement = "<"; return true; }
	if (entity == "gt") { replacement = ">"; return true; }
	if (entity == "quot") { replacement = "\""; return true; }
	if (entity == "apos") { replacement = "'"; return true; }
	if (entity == "nbsp") { replacement = " "; return true; }
	if (entity == "ldquo" || entity == "rdquo" || entity == "laquo" || entity == "raquo") { replacement = "\""; return true; }
	if (entity == "lsquo" || entity == "rsquo" || entity == "sbquo") { replacement = "'"; return true; }
	if (entity == "ndash" || entity == "minus") { replacement = "-"; return true; }
	if (entity == "mdash") { replacement = "--"; return true; }
	if (entity == "hellip") { replacement = "..."; return true; }

	return false;
}

void normalizeSmartPunctuationAscii(String& text)
{
	text.replace("\xE2\x80\x98", "'");
	text.replace("\xE2\x80\x99", "'");
	text.replace("\xE2\x80\x9A", "'");
	text.replace("\xE2\x80\x9B", "'");
	text.replace("\xE2\x80\x9C", "\"");
	text.replace("\xE2\x80\x9D", "\"");
	text.replace("\xE2\x80\x9E", "\"");
	text.replace("\xE2\x80\x9F", "\"");
	text.replace("\xE2\x80\x93", "-");
	text.replace("\xE2\x80\x94", "--");
	text.replace("\xE2\x80\xA6", "...");
	text.replace("\xE2\x88\x92", "-");
	text.replace("\xC2\xA0", " ");
}

String encodeStyledLine(const String& text, ReaderFontStyle style);
ReaderTextAlign resolveTagAlignment(const String& attrs, bool& hasAlignment);
bool appendNextReaderPage();
String stripZipPathSuffix(const String& path);
void goToReaderPage(int32_t pageIndex);
void showStatusOnDisplay(const String& title, const String& line);
void showMainMenu();
void showInvalidOptionOnDisplay();
uint32_t computeReaderIndexSignature(const String& normalizedPath, const std::vector<String>& chapterPaths);
bool tryLoadReaderIndexCache(uint32_t signature, uint16_t chapterCount);
void saveReaderIndexCache(uint32_t signature, uint16_t chapterCount, uint32_t totalPages);

uint16_t mainMenuOptionCount()
{
	return static_cast<uint16_t>(3 + bookCount);
}

uint16_t deleteMenuOptionCount()
{
	return static_cast<uint16_t>(1 + bookCount);
}

uint16_t settingsMenuOptionCount()
{
	return 11;
}

uint16_t readerStartMenuOptionCount()
{
	return static_cast<uint16_t>(3 + readerChapterPaths.size());
}

void clampMenuCursor(uint16_t& cursor, uint16_t optionCount)
{
	if (optionCount == 0)
	{
		cursor = 0;
		return;
	}
	if (cursor >= optionCount)
	{
		cursor = static_cast<uint16_t>(optionCount - 1);
	}
}

void moveMenuCursor(uint16_t& cursor, uint16_t optionCount, int8_t delta)
{
	if (optionCount == 0)
	{
		cursor = 0;
		return;
	}
	int32_t next = static_cast<int32_t>(cursor) + delta;
	if (next < 0)
	{
		next = static_cast<int32_t>(optionCount) - 1;
	}
	else if (next >= static_cast<int32_t>(optionCount))
	{
		next = 0;
	}
	cursor = static_cast<uint16_t>(next);
}

SerialNavInput parseSerialNavInput(String input)
{
	input.trim();
	input.toLowerCase();

	if (input == "up" || input == "u")
	{
		return SerialNavInput::Up;
	}
	if (input == "down" || input == "d")
	{
		return SerialNavInput::Down;
	}
	if (input == "left" || input == "l" || input == "back")
	{
		return SerialNavInput::Left;
	}
	if (input == "right" || input == "r" || input == "select" || input == "ok")
	{
		return SerialNavInput::Right;
	}

	return SerialNavInput::None;
}

void drawMenuCursorTriangle(int16_t baselineY)
{
	const int16_t tipX = 14;
	const int16_t centerY = static_cast<int16_t>(baselineY - 5);
	const int16_t halfHeight = 4;
	display.fillTriangle(tipX, centerY, static_cast<int16_t>(tipX - 8), static_cast<int16_t>(centerY - halfHeight), static_cast<int16_t>(tipX - 8), static_cast<int16_t>(centerY + halfHeight), GxEPD_BLACK);
}

void printSerialMenuLine(bool selected, const String& text)
{
	Serial.print(selected ? "> " : "  ");
	Serial.println(text);
}

uint32_t fnv1aUpdate(uint32_t hash, const uint8_t* data, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		hash ^= data[i];
		hash *= 16777619UL;
	}
	return hash;
}

uint32_t fnv1aUpdateString(uint32_t hash, const String& value)
{
	return fnv1aUpdate(hash, reinterpret_cast<const uint8_t*>(value.c_str()), value.length());
}

uint32_t computeReaderIndexSignature(const String& normalizedPath, const std::vector<String>& chapterPaths)
{
	uint32_t hash = 2166136261UL;
	hash = fnv1aUpdateString(hash, normalizedPath);

	File f = LittleFS.open(normalizedPath, "r");
	const uint32_t size = f ? static_cast<uint32_t>(f.size()) : 0;
	if (f)
	{
		f.close();
	}
	hash = fnv1aUpdate(hash, reinterpret_cast<const uint8_t*>(&size), sizeof(size));

	const uint8_t family = static_cast<uint8_t>(readerFontFamily);
	hash = fnv1aUpdate(hash, &family, sizeof(family));
	hash = fnv1aUpdate(hash, &readerFontSizePt, sizeof(readerFontSizePt));
	const uint8_t spacing = static_cast<uint8_t>(readerLineSpacing);
	hash = fnv1aUpdate(hash, &spacing, sizeof(spacing));

	const uint16_t chapterCount = static_cast<uint16_t>(chapterPaths.size());
	hash = fnv1aUpdate(hash, reinterpret_cast<const uint8_t*>(&chapterCount), sizeof(chapterCount));
	for (const String& chapterPath : chapterPaths)
	{
		hash = fnv1aUpdateString(hash, chapterPath);
	}

	return hash;
}

void makeIndexCacheKey(char* outKey, size_t outKeySize, char prefix, uint8_t slot)
{
	if (!outKey || outKeySize == 0)
	{
		return;
	}
	snprintf(outKey, outKeySize, "%c%u", prefix, slot);
}

bool tryLoadReaderIndexCache(uint32_t signature, uint16_t chapterCount)
{
	if (chapterCount == 0 || chapterCount > kMaxBooks)
	{
		return false;
	}

	Preferences cache;
	if (!cache.begin(kIndexCacheNamespace, true))
	{
		return false;
	}

	const size_t bytesExpected = static_cast<size_t>(chapterCount) * sizeof(uint16_t);
	for (uint8_t slot = 0; slot < kIndexCacheSlots; ++slot)
	{
		char keySig[4];
		char keyCount[4];
		char keyTotal[4];
		char keyCounts[4];
		char keyUse[4];
		makeIndexCacheKey(keySig, sizeof(keySig), 's', slot);
		makeIndexCacheKey(keyCount, sizeof(keyCount), 'c', slot);
		makeIndexCacheKey(keyTotal, sizeof(keyTotal), 't', slot);
		makeIndexCacheKey(keyCounts, sizeof(keyCounts), 'b', slot);
		makeIndexCacheKey(keyUse, sizeof(keyUse), 'u', slot);

		const uint32_t storedSig = cache.getULong(keySig, 0);
		const uint8_t storedCount = cache.getUChar(keyCount, 0);
		if (storedSig != signature || storedCount != chapterCount)
		{
			continue;
		}

		const size_t storedBytes = cache.getBytesLength(keyCounts);
		if (storedBytes != bytesExpected)
		{
			continue;
		}

		for (uint16_t i = 0; i < kMaxBooks; ++i)
		{
			readerChapterPageCounts[i] = 0;
		}

		if (cache.getBytes(keyCounts, readerChapterPageCounts, bytesExpected) != bytesExpected)
		{
			continue;
		}

		readerTotalPagesLoaded = cache.getULong(keyTotal, 0);
		const uint32_t seq = cache.getULong("seq", 0);
		cache.putULong(keyUse, seq + 1);
		cache.putULong("seq", seq + 1);
		cache.end();
		return readerTotalPagesLoaded > 0;
	}

	cache.end();
	return false;
}

void saveReaderIndexCache(uint32_t signature, uint16_t chapterCount, uint32_t totalPages)
{
	if (chapterCount == 0 || chapterCount > kMaxBooks)
	{
		return;
	}

	Preferences cache;
	if (!cache.begin(kIndexCacheNamespace, false))
	{
		return;
	}

	int8_t targetSlot = -1;
	uint32_t oldestUse = 0xFFFFFFFFUL;
	for (uint8_t slot = 0; slot < kIndexCacheSlots; ++slot)
	{
		char keySig[4];
		char keyCount[4];
		char keyUse[4];
		makeIndexCacheKey(keySig, sizeof(keySig), 's', slot);
		makeIndexCacheKey(keyCount, sizeof(keyCount), 'c', slot);
		makeIndexCacheKey(keyUse, sizeof(keyUse), 'u', slot);

		const uint32_t storedSig = cache.getULong(keySig, 0);
		const uint8_t storedCount = cache.getUChar(keyCount, 0);
		if (storedSig == signature && storedCount == chapterCount)
		{
			targetSlot = static_cast<int8_t>(slot);
			break;
		}

		if (storedSig == 0 || storedCount == 0)
		{
			targetSlot = static_cast<int8_t>(slot);
			oldestUse = 0;
			continue;
		}

		if (targetSlot >= 0 && oldestUse == 0)
		{
			continue;
		}

		const uint32_t use = cache.getULong(keyUse, 0);
		if (targetSlot < 0 || use < oldestUse)
		{
			targetSlot = static_cast<int8_t>(slot);
			oldestUse = use;
		}
	}

	if (targetSlot < 0)
	{
		targetSlot = 0;
	}

	char keySig[4];
	char keyCount[4];
	char keyTotal[4];
	char keyCounts[4];
	char keyUse[4];
	makeIndexCacheKey(keySig, sizeof(keySig), 's', static_cast<uint8_t>(targetSlot));
	makeIndexCacheKey(keyCount, sizeof(keyCount), 'c', static_cast<uint8_t>(targetSlot));
	makeIndexCacheKey(keyTotal, sizeof(keyTotal), 't', static_cast<uint8_t>(targetSlot));
	makeIndexCacheKey(keyCounts, sizeof(keyCounts), 'b', static_cast<uint8_t>(targetSlot));
	makeIndexCacheKey(keyUse, sizeof(keyUse), 'u', static_cast<uint8_t>(targetSlot));

	const uint32_t seq = cache.getULong("seq", 0) + 1;
	cache.putULong(keySig, signature);
	cache.putUChar(keyCount, static_cast<uint8_t>(chapterCount));
	cache.putULong(keyTotal, totalPages);
	cache.putULong(keyUse, seq);
	cache.putULong("seq", seq);
	const size_t bytes = static_cast<size_t>(chapterCount) * sizeof(uint16_t);
	cache.putBytes(keyCounts, readerChapterPageCounts, bytes);
	cache.end();
}

void resetReaderMarkupStreamState()
{
	readerMarkupState = ReaderMarkupStreamState();
	readerParsedTextBuffer = "";
	readerMarkupTextSegment = "";
	readerMarkupRawCarry = "";
	readerAwaitingStyleCode = false;
	readerAwaitingAlignCode = false;
	readerChapterInputEnded = true;
}

void resetReaderPageBuilderState()
{
	readerPages.clear();
	readerPageIndex = 0;
	readerParsePos = 0;
	readerParseCurrentLine = "";
	readerParseCurrentPage = "";
	readerParseWord = "";
	readerParseCurrentStyle = ReaderFontStyle::Normal;
	readerParseCurrentAlign = ReaderTextAlign::Left;
	readerParseLineCount = 0;
	readerChapterParseComplete = false;
}

void flushReaderMarkupTextSegment()
{
	if (readerMarkupTextSegment.isEmpty())
	{
		return;
	}

	String normalized = readerMarkupTextSegment;
	normalized.replace("\r", "");
	normalizeSmartPunctuationAscii(normalized);
	readerParsedTextBuffer += normalized;
	readerMarkupTextSegment = "";
}

bool pushReaderPageLine(const String& encodedLine)
{
	if (readerParseCurrentPage.length() > 0)
	{
		readerParseCurrentPage += '\n';
	}
	readerParseCurrentPage += encodeAlignedLine(encodedLine, readerParseCurrentAlign);
	++readerParseLineCount;
	const int32_t usableHeight = static_cast<int32_t>(display.height()) - static_cast<int32_t>(uiReaderBodyStartY()) - 8;
	uint16_t maxLinesPerPage = 1;
	if (usableHeight > 0)
	{
		maxLinesPerPage = static_cast<uint16_t>(usableHeight / static_cast<int32_t>(uiLineStep()));
		if (maxLinesPerPage == 0)
		{
			maxLinesPerPage = 1;
		}
	}

	if (readerParseLineCount >= maxLinesPerPage)
	{
		readerPages.push_back(readerParseCurrentPage);
		readerParseCurrentPage = "";
		readerParseLineCount = 0;
		return true;
	}
	return false;
}

bool flushReaderPageWord()
{
	if (readerParseWord.isEmpty())
	{
		return false;
	}

	String pendingWord = readerParseWord;
	readerParseWord = "";

	const uint16_t usableWidth = display.width() - 16;

	while (!pendingWord.isEmpty())
	{
		String candidate = readerParseCurrentLine;
		if (!candidate.isEmpty())
		{
			candidate += ' ';
		}

		if (trailingStyleForEncodedText(candidate) != readerParseCurrentStyle)
		{
			appendStyleMarker(candidate, readerParseCurrentStyle);
		}
		candidate += pendingWord;

		if (measureEncodedTextWidth(candidate) <= usableWidth)
		{
			readerParseCurrentLine = candidate;
			pendingWord = "";
			break;
		}

		if (!readerParseCurrentLine.isEmpty())
		{
			if (pushReaderPageLine(readerParseCurrentLine))
			{
				readerParseWord = pendingWord;
				readerParseCurrentLine = "";
				return true;
			}
			readerParseCurrentLine = "";
			continue;
		}

		size_t fitLen = 0;
		for (size_t len = 1; len <= pendingWord.length(); ++len)
		{
			String chunkEncoded;
			if (readerParseCurrentStyle != ReaderFontStyle::Normal)
			{
				appendStyleMarker(chunkEncoded, readerParseCurrentStyle);
			}
			chunkEncoded += pendingWord.substring(0, len);
			if (measureEncodedTextWidth(chunkEncoded) <= usableWidth)
			{
				fitLen = len;
			}
			else
			{
				break;
			}
		}

		if (fitLen == 0)
		{
			fitLen = 1;
		}

		String chunkEncoded;
		if (readerParseCurrentStyle != ReaderFontStyle::Normal)
		{
			appendStyleMarker(chunkEncoded, readerParseCurrentStyle);
		}
		chunkEncoded += pendingWord.substring(0, fitLen);
		pendingWord = pendingWord.substring(fitLen);

		if (pendingWord.isEmpty())
		{
			readerParseCurrentLine = chunkEncoded;
			break;
		}

		if (pushReaderPageLine(chunkEncoded))
		{
			readerParseWord = pendingWord;
			readerParseCurrentLine = "";
			return true;
		}
	}

	return false;
}

bool consumeReaderParsedChar(char ch)
{
	if (readerAwaitingAlignCode)
	{
		readerAwaitingAlignCode = false;
		if (isAlignCodeChar(ch))
		{
			readerParseCurrentAlign = alignFromCodeChar(ch);
		}
		return false;
	}

	if (readerAwaitingStyleCode)
	{
		readerAwaitingStyleCode = false;
		if (isStyleCodeChar(ch))
		{
			const ReaderFontStyle nextStyle = styleFromCodeChar(ch);
			if (!readerParseCurrentLine.isEmpty() && trailingStyleForEncodedText(readerParseCurrentLine) != nextStyle)
			{
				appendStyleMarker(readerParseCurrentLine, nextStyle);
			}
			readerParseCurrentStyle = nextStyle;
		}
		return false;
	}

	if (ch == kAlignMarker)
	{
		if (flushReaderPageWord())
		{
			return true;
		}
		if (!readerParseCurrentLine.isEmpty())
		{
			if (pushReaderPageLine(readerParseCurrentLine))
			{
				readerParseCurrentLine = "";
				return true;
			}
			readerParseCurrentLine = "";
		}
		readerAwaitingAlignCode = true;
		return false;
	}

	if (ch == kStyleMarker)
	{
		if (flushReaderPageWord())
		{
			return true;
		}
		readerAwaitingStyleCode = true;
		return false;
	}

	if (ch == '\n')
	{
		if (flushReaderPageWord())
		{
			return true;
		}
		if (readerParseCurrentLine.length() > 0)
		{
			if (pushReaderPageLine(readerParseCurrentLine))
			{
				readerParseCurrentLine = "";
				return true;
			}
			readerParseCurrentLine = "";
		}
		if (readerParseLineCount > 0)
		{
			if (pushReaderPageLine(""))
			{
				return true;
			}
		}
		return false;
	}

	if (isspace(static_cast<unsigned char>(ch)))
	{
		return flushReaderPageWord();
	}

	readerParseWord += ch;
	return false;
}

bool drainReaderParsedTextBuffer()
{
	size_t index = 0;
	while (index < readerParsedTextBuffer.length())
	{
		if ((index & 0x1FF) == 0)
		{
			yield();
		}

		if (consumeReaderParsedChar(readerParsedTextBuffer[index]))
		{
			readerParsedTextBuffer = readerParsedTextBuffer.substring(index + 1);
			return true;
		}
		++index;
	}

	readerParsedTextBuffer = "";
	return false;
}

void finalizeReaderPageBuilder()
{
	if (flushReaderPageWord())
	{
		return;
	}

	if (readerParseCurrentLine.length() > 0)
	{
		if (pushReaderPageLine(readerParseCurrentLine))
		{
			readerParseCurrentLine = "";
			return;
		}
		readerParseCurrentLine = "";
	}

	if (readerParseCurrentPage.length() > 0)
	{
		readerPages.push_back(readerParseCurrentPage);
		readerParseCurrentPage = "";
		readerParseLineCount = 0;
	}

	readerChapterParseComplete = true;
	if (readerPages.empty())
	{
		readerPages.push_back(encodeStyledLine("(No readable text found)", ReaderFontStyle::Oblique));
	}
}

void feedReaderMarkupByte(char ch)
{
	if (readerMarkupState.inEntity)
	{
		if (ch == ';')
		{
			String replacement;
			if (decodeEntityToAscii(readerMarkupState.entityBuffer, replacement))
			{
				readerMarkupTextSegment += replacement;
			}
			else
			{
				readerMarkupTextSegment += '&';
				readerMarkupTextSegment += readerMarkupState.entityBuffer;
				readerMarkupTextSegment += ';';
			}
			readerMarkupState.inEntity = false;
			readerMarkupState.entityBuffer = "";
		}
		else if (readerMarkupState.entityBuffer.length() < 12)
		{
			readerMarkupState.entityBuffer += ch;
		}
		else
		{
			readerMarkupTextSegment += '&';
			readerMarkupTextSegment += readerMarkupState.entityBuffer;
			readerMarkupTextSegment += ch;
			readerMarkupState.inEntity = false;
			readerMarkupState.entityBuffer = "";
		}
		return;
	}

	if (readerMarkupState.inTag)
	{
		if (ch == '>')
		{
			readerMarkupState.inTag = false;
			if (readerMarkupState.tagLen > 0)
			{
				const bool isCloseTag = readerMarkupState.tagName[0] == '/';
				const char* normalizedTag = isCloseTag ? readerMarkupState.tagName + 1 : readerMarkupState.tagName;
				const bool isBlockTag = strcmp(normalizedTag, "p") == 0 || strcmp(normalizedTag, "div") == 0 || strcmp(normalizedTag, "section") == 0 || strcmp(normalizedTag, "article") == 0 || strcmp(normalizedTag, "li") == 0 || strcmp(normalizedTag, "blockquote") == 0 || strcmp(normalizedTag, "h1") == 0 || strcmp(normalizedTag, "h2") == 0 || strcmp(normalizedTag, "h3") == 0 || strcmp(normalizedTag, "h4") == 0 || strcmp(normalizedTag, "h5") == 0 || strcmp(normalizedTag, "h6") == 0;
				const bool isSpanLikeTag = strcmp(normalizedTag, "span") == 0 || strcmp(normalizedTag, "a") == 0 || strcmp(normalizedTag, "p") == 0 || strcmp(normalizedTag, "div") == 0;
				const bool attrItalic = strstr(readerMarkupState.tagAttrs, "italic") || strstr(readerMarkupState.tagAttrs, "oblique") || strstr(readerMarkupState.tagAttrs, "emphasis");
				const bool attrBold = strstr(readerMarkupState.tagAttrs, "bold") || strstr(readerMarkupState.tagAttrs, "font-weight:700") || strstr(readerMarkupState.tagAttrs, "font-weight:800") || strstr(readerMarkupState.tagAttrs, "font-weight:900");
				const bool isBreakTag = strcmp(normalizedTag, "br") == 0 || strcmp(normalizedTag, "p") == 0 || strcmp(normalizedTag, "div") == 0 || strcmp(normalizedTag, "section") == 0 || strcmp(normalizedTag, "article") == 0 || strcmp(normalizedTag, "li") == 0;
				const bool isHeaderTag = strcmp(normalizedTag, "h1") == 0 || strcmp(normalizedTag, "h2") == 0 || strcmp(normalizedTag, "h3") == 0 || strcmp(normalizedTag, "h4") == 0 || strcmp(normalizedTag, "h5") == 0 || strcmp(normalizedTag, "h6") == 0;

				if (!isCloseTag && (strcmp(normalizedTag, "strong") == 0 || strcmp(normalizedTag, "b") == 0))
				{
					++readerMarkupState.boldDepth;
				}
				else if (isCloseTag && (strcmp(normalizedTag, "strong") == 0 || strcmp(normalizedTag, "b") == 0) && readerMarkupState.boldDepth > 0)
				{
					--readerMarkupState.boldDepth;
				}

				if (!isCloseTag && (strcmp(normalizedTag, "em") == 0 || strcmp(normalizedTag, "i") == 0))
				{
					++readerMarkupState.obliqueDepth;
				}
				else if (isCloseTag && (strcmp(normalizedTag, "em") == 0 || strcmp(normalizedTag, "i") == 0) && readerMarkupState.obliqueDepth > 0)
				{
					--readerMarkupState.obliqueDepth;
				}

				if (!isCloseTag && isSpanLikeTag)
				{
					if (attrBold)
					{
						++readerMarkupState.attrBoldDepth;
					}
					if (attrItalic)
					{
						++readerMarkupState.attrObliqueDepth;
					}
				}
				else if (isCloseTag && isSpanLikeTag)
				{
					if (readerMarkupState.attrBoldDepth > 0)
					{
						--readerMarkupState.attrBoldDepth;
					}
					if (readerMarkupState.attrObliqueDepth > 0)
					{
						--readerMarkupState.attrObliqueDepth;
					}
				}

				if (!isCloseTag && isHeaderTag)
				{
					++readerMarkupState.headerDepth;
				}
				else if (isCloseTag && isHeaderTag && readerMarkupState.headerDepth > 0)
				{
					--readerMarkupState.headerDepth;
				}

				if (isBreakTag || isHeaderTag)
				{
					flushReaderMarkupTextSegment();
					if (!readerParsedTextBuffer.isEmpty() && readerParsedTextBuffer[readerParsedTextBuffer.length() - 1] != '\n')
					{
						readerParsedTextBuffer += '\n';
					}
				}

				if (!isCloseTag && isBlockTag)
				{
					flushReaderMarkupTextSegment();
					bool hasAlignment = false;
					const ReaderTextAlign resolvedAlign = resolveTagAlignment(String(readerMarkupState.tagAttrs), hasAlignment);
					appendAlignMarker(readerParsedTextBuffer, hasAlignment ? resolvedAlign : ReaderTextAlign::Left);
					readerMarkupState.activeAlign = hasAlignment ? resolvedAlign : ReaderTextAlign::Left;
				}

				const ReaderFontStyle desiredStyle = [&]() -> ReaderFontStyle
				{
					const bool bold = (readerMarkupState.boldDepth > 0) || (readerMarkupState.headerDepth > 0) || (readerMarkupState.attrBoldDepth > 0);
					const bool oblique = (readerMarkupState.obliqueDepth > 0) || (readerMarkupState.attrObliqueDepth > 0);
					if (bold && oblique)
					{
						return ReaderFontStyle::BoldOblique;
					}
					if (bold)
					{
						return ReaderFontStyle::Bold;
					}
					if (oblique)
					{
						return ReaderFontStyle::Oblique;
					}
					return ReaderFontStyle::Normal;
				}();
				if (desiredStyle != readerMarkupState.activeStyle)
				{
					flushReaderMarkupTextSegment();
					appendStyleMarker(readerParsedTextBuffer, desiredStyle);
					readerMarkupState.activeStyle = desiredStyle;
				}
			}
			readerMarkupState.tagLen = 0;
			readerMarkupState.attrsLen = 0;
			readerMarkupState.tagName[0] = '\0';
			readerMarkupState.tagAttrs[0] = '\0';
			readerMarkupState.collectingTagName = false;
			readerMarkupState.sawTagNameChar = false;
			return;
		}

		if (readerMarkupState.collectingTagName)
		{
			if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
			{
				readerMarkupState.collectingTagName = false;
				return;
			}

			if (!readerMarkupState.sawTagNameChar && (ch == '!' || ch == '?'))
			{
				readerMarkupState.collectingTagName = false;
				return;
			}

			if (readerMarkupState.tagLen < sizeof(readerMarkupState.tagName) - 1)
			{
				readerMarkupState.tagName[readerMarkupState.tagLen++] = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
				readerMarkupState.tagName[readerMarkupState.tagLen] = '\0';
				readerMarkupState.sawTagNameChar = true;
			}
			return;
		}

		if (readerMarkupState.attrsLen < sizeof(readerMarkupState.tagAttrs) - 1)
		{
			readerMarkupState.tagAttrs[readerMarkupState.attrsLen++] = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
			readerMarkupState.tagAttrs[readerMarkupState.attrsLen] = '\0';
		}
		return;
	}

	if (ch == '<')
	{
		flushReaderMarkupTextSegment();
		readerMarkupState.inTag = true;
		readerMarkupState.tagLen = 0;
		readerMarkupState.attrsLen = 0;
		readerMarkupState.tagAttrs[0] = '\0';
		readerMarkupState.collectingTagName = true;
		readerMarkupState.sawTagNameChar = false;
		return;
	}

	if (ch == '&')
	{
		readerMarkupState.inEntity = true;
		readerMarkupState.entityBuffer = "";
		return;
	}

	readerMarkupTextSegment += ch;
}

void processReaderMarkupBuffer(bool finalChunk)
{
	String combined = readerMarkupRawCarry;
	if (!finalChunk && combined.length() <= 2)
	{
		return;
	}

	if (!combined.isEmpty() && !finalChunk)
	{
		const size_t processLength = combined.length() > 2 ? combined.length() - 2 : 0;
		for (size_t i = 0; i < processLength; ++i)
		{
			if ((i & 0x1FF) == 0)
			{
				yield();
			}

			feedReaderMarkupByte(combined[i]);
		}
		readerMarkupRawCarry = combined.substring(processLength);
	}
	else
	{
		for (size_t i = 0; i < combined.length(); ++i)
		{
			if ((i & 0x1FF) == 0)
			{
				yield();
			}

			feedReaderMarkupByte(combined[i]);
		}
		readerMarkupRawCarry = "";
	}

	flushReaderMarkupTextSegment();
}

void appendUniqueClassName(std::vector<String>& classes, const String& className)
{
	if (className.isEmpty())
	{
		return;
	}
	for (const String& existing : classes)
	{
		if (existing == className)
		{
			return;
		}
	}
	classes.push_back(className);
}

void collectCssAlignmentClasses(const String& cssText)
{
	int pos = 0;
	while (pos >= 0 && pos < static_cast<int>(cssText.length()))
	{
		const int dot = cssText.indexOf('.', pos);
		if (dot < 0)
		{
			break;
		}

		int classStart = dot + 1;
		int classEnd = classStart;
		while (classEnd < static_cast<int>(cssText.length()))
		{
			const char ch = cssText[classEnd];
			if (isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-')
			{
				++classEnd;
			}
			else
			{
				break;
			}
		}

		if (classEnd <= classStart)
		{
			pos = dot + 1;
			continue;
		}

		const int openBrace = cssText.indexOf('{', classEnd);
		if (openBrace < 0)
		{
			break;
		}
		const int closeBrace = cssText.indexOf('}', openBrace + 1);
		if (closeBrace < 0)
		{
			break;
		}

		String block = cssText.substring(openBrace + 1, closeBrace);
		block.toLowerCase();
		String compact = block;
		compact.replace(" ", "");
		compact.replace("\t", "");
		compact.replace("\r", "");
		compact.replace("\n", "");
		String className = cssText.substring(classStart, classEnd);
		className.toLowerCase();

		if (compact.indexOf("text-align:center") >= 0)
		{
			appendUniqueClassName(readerCssAlignCenterClasses, className);
		}
		else if (compact.indexOf("text-align:right") >= 0)
		{
			appendUniqueClassName(readerCssAlignRightClasses, className);
		}

		pos = closeBrace + 1;
	}
}

bool attributeHasClass(const String& attrs, const String& wantedClass)
{
	if (attrs.isEmpty() || wantedClass.isEmpty())
	{
		return false;
	}

	const int classPos = attrs.indexOf("class=");
	if (classPos < 0)
	{
		return false;
	}

	int valueStart = classPos + 6;
	if (valueStart >= static_cast<int>(attrs.length()))
	{
		return false;
	}

	const char quote = attrs[valueStart];
	int valueEnd = -1;
	if (quote == '"' || quote == '\'')
	{
		++valueStart;
		valueEnd = attrs.indexOf(quote, valueStart);
	}
	else
	{
		valueEnd = attrs.indexOf(' ', valueStart);
		if (valueEnd < 0)
		{
			valueEnd = static_cast<int>(attrs.length());
		}
	}

	if (valueEnd <= valueStart)
	{
		return false;
	}

	String classList = attrs.substring(valueStart, valueEnd);
	classList.trim();
	int tokenStart = 0;
	while (tokenStart <= static_cast<int>(classList.length()))
	{
		while (tokenStart < static_cast<int>(classList.length()) && isspace(static_cast<unsigned char>(classList[tokenStart])))
		{
			++tokenStart;
		}
		if (tokenStart >= static_cast<int>(classList.length()))
		{
			break;
		}

		int tokenEnd = tokenStart;
		while (tokenEnd < static_cast<int>(classList.length()) && !isspace(static_cast<unsigned char>(classList[tokenEnd])))
		{
			++tokenEnd;
		}

		String token = classList.substring(tokenStart, tokenEnd);
		token.toLowerCase();
		if (token == wantedClass)
		{
			return true;
		}
		tokenStart = tokenEnd + 1;
	}

	return false;
}

ReaderTextAlign resolveTagAlignment(const String& attrs, bool& hasAlignment)
{
	hasAlignment = false;
	String lowered = attrs;
	lowered.toLowerCase();

	if (lowered.indexOf("text-align:center") >= 0 || lowered.indexOf("align=\"center\"") >= 0 || lowered.indexOf("align='center'") >= 0)
	{
		hasAlignment = true;
		return ReaderTextAlign::Center;
	}
	if (lowered.indexOf("text-align:right") >= 0 || lowered.indexOf("align=\"right\"") >= 0 || lowered.indexOf("align='right'") >= 0)
	{
		hasAlignment = true;
		return ReaderTextAlign::Right;
	}
	if (lowered.indexOf("text-align:left") >= 0 || lowered.indexOf("align=\"left\"") >= 0 || lowered.indexOf("align='left'") >= 0)
	{
		hasAlignment = true;
		return ReaderTextAlign::Left;
	}

	for (const String& className : readerCssAlignCenterClasses)
	{
		if (attributeHasClass(lowered, className))
		{
			hasAlignment = true;
			return ReaderTextAlign::Center;
		}
	}
	for (const String& className : readerCssAlignRightClasses)
	{
		if (attributeHasClass(lowered, className))
		{
			hasAlignment = true;
			return ReaderTextAlign::Right;
		}
	}

	return ReaderTextAlign::Left;
}

String encodeStyledLine(const String& text, ReaderFontStyle style)
{
	if (style == ReaderFontStyle::Normal)
	{
		return text;
	}

	String encoded;
	encoded.reserve(text.length() + 2);
	appendStyleMarker(encoded, style);
	encoded += text;
	return encoded;
}

String htmlEscape(const String& input)
{
	String output;
	output.reserve(input.length() + 16);
	for (size_t i = 0; i < input.length(); ++i)
	{
		const char ch = input[i];
		switch (ch)
		{
			case '&': output += "&amp;"; break;
			case '<': output += "&lt;"; break;
			case '>': output += "&gt;"; break;
			case '"': output += "&quot;"; break;
			case '\'': output += "&#39;"; break;
			default: output += ch; break;
		}
	}
	return output;
}

String sanitizeFilename(String filename)
{
	filename.replace("\\", "/");
	const int lastSlash = filename.lastIndexOf('/');
	if (lastSlash >= 0)
	{
		filename = filename.substring(lastSlash + 1);
	}
	filename.trim();
	filename.replace("..", "_");
	filename.replace(" ", "_");
	if (filename.isEmpty())
	{
		filename = "book.bin";
	}
	if (!filename.startsWith("/"))
	{
		filename = "/" + filename;
	}
	return filename;
}

String truncateForDisplay(const String& text, size_t maxCharacters)
{
	if (text.length() <= maxCharacters)
	{
		return text;
	}
	if (maxCharacters <= 3)
	{
		return text.substring(0, maxCharacters);
	}
	return text.substring(0, maxCharacters - 3) + "...";
}

int16_t findReaderBookmarkIndex(const String& normalizedBookPath)
{
	for (uint16_t i = 0; i < readerBookmarkCount; ++i)
	{
		if (readerBookmarks[i].bookPath == normalizedBookPath)
		{
			return static_cast<int16_t>(i);
		}
	}
	return -1;
}

void saveReaderBookmark(const String& normalizedBookPath, uint16_t chapterIndex, uint16_t pageIndex, bool showingCover)
{
	if (normalizedBookPath.isEmpty())
	{
		return;
	}

	const int16_t existingIndex = findReaderBookmarkIndex(normalizedBookPath);
	if (existingIndex >= 0)
	{
		ReaderBookmark& bookmark = readerBookmarks[existingIndex];
		bookmark.chapterIndex = chapterIndex;
		bookmark.pageIndex = pageIndex;
		bookmark.showingCover = showingCover;
		readerBookmarksDirty = true;
		return;
	}

	if (readerBookmarkCount < kMaxBooks)
	{
		ReaderBookmark& bookmark = readerBookmarks[readerBookmarkCount++];
		bookmark.bookPath = normalizedBookPath;
		bookmark.chapterIndex = chapterIndex;
		bookmark.pageIndex = pageIndex;
		bookmark.showingCover = showingCover;
		readerBookmarksDirty = true;
		return;
	}

	for (uint16_t i = 1; i < kMaxBooks; ++i)
	{
		readerBookmarks[i - 1] = readerBookmarks[i];
	}
	ReaderBookmark& tail = readerBookmarks[kMaxBooks - 1];
	tail.bookPath = normalizedBookPath;
	tail.chapterIndex = chapterIndex;
	tail.pageIndex = pageIndex;
	tail.showingCover = showingCover;
	readerBookmarksDirty = true;
}

bool loadReaderBookmark(const String& normalizedBookPath, uint16_t& chapterIndex, uint16_t& pageIndex, bool& showingCover)
{
	const int16_t index = findReaderBookmarkIndex(normalizedBookPath);
	if (index < 0)
	{
		return false;
	}

	const ReaderBookmark& bookmark = readerBookmarks[index];
	chapterIndex = bookmark.chapterIndex;
	pageIndex = bookmark.pageIndex;
	showingCover = bookmark.showingCover;
	return true;
}

String chapterLabelForDisplay(uint16_t chapterIndex)
{
	if (chapterIndex < readerChapterTitles.size() && !readerChapterTitles[chapterIndex].isEmpty())
	{
		return readerChapterTitles[chapterIndex];
	}

	if (chapterIndex >= readerChapterPaths.size())
	{
		return "Untitled chapter";
	}

	String path = stripZipPathSuffix(readerChapterPaths[chapterIndex]);
	const int slash = path.lastIndexOf('/');
	if (slash >= 0)
	{
		path = path.substring(slash + 1);
	}
	if (path.isEmpty())
	{
		return "Untitled chapter";
	}

	const int dot = path.lastIndexOf('.');
	if (dot > 0)
	{
		path = path.substring(0, dot);
	}
	path.replace('_', ' ');
	path.trim();
	if (path.isEmpty())
	{
		return "Untitled chapter";
	}
	return path;
}

uint32_t globalPageForPosition(uint16_t chapterIndex, uint16_t pageIndex, bool showingCover)
{
	if (showingCover && readerHasCover)
	{
		return 0;
	}

	uint32_t page = 0;
	for (uint16_t i = 0; i < chapterIndex && i < kMaxBooks; ++i)
	{
		page += readerChapterPageCounts[i];
	}

	page += static_cast<uint32_t>(pageIndex) + 1U;
	return page;
}

String normalizeZipPath(const String& path);
String stripZipPathSuffix(const String& path)
{
	int cut = path.indexOf('#');
	const int query = path.indexOf('?');
	if (query >= 0 && (cut < 0 || query < cut))
	{
		cut = query;
	}
	if (cut >= 0)
	{
		return path.substring(0, cut);
	}
	return path;
}

String resolveRelativePath(const String& basePath, const String& relativePath)
{
	const String cleanedRelativePath = stripZipPathSuffix(relativePath);
	if (cleanedRelativePath.startsWith("/"))
	{
		return normalizeZipPath(cleanedRelativePath);
	}

	const int lastSlash = basePath.lastIndexOf('/');
	if (lastSlash < 0)
	{
		return normalizeZipPath("/" + cleanedRelativePath);
	}
	return normalizeZipPath(basePath.substring(0, lastSlash + 1) + cleanedRelativePath);
}

String normalizeZipPath(const String& path)
{
	std::vector<String> segments;
	segments.reserve(8);

	bool absolute = path.startsWith("/");
	int start = 0;
	while (start <= static_cast<int>(path.length()))
	{
		const int slash = path.indexOf('/', start);
		const int end = (slash >= 0) ? slash : static_cast<int>(path.length());
		String segment = path.substring(start, end);
		if (segment.length() > 0 && segment != ".")
		{
			if (segment == "..")
			{
				if (!segments.empty())
				{
					segments.pop_back();
				}
			}
			else
			{
				segments.push_back(segment);
			}
		}
		if (slash < 0)
		{
			break;
		}
		start = slash + 1;
	}

	String normalized = absolute ? "/" : "";
	for (size_t i = 0; i < segments.size(); ++i)
	{
		if (!normalized.isEmpty() && !normalized.endsWith("/"))
		{
			normalized += "/";
		}
		normalized += segments[i];
	}
	return normalized;
}
bool hasTextExtension(const String& path)
{
	const String lowerPath = String(path);
	return lowerPath.endsWith(".xhtml") || lowerPath.endsWith(".html") || lowerPath.endsWith(".htm") || lowerPath.endsWith(".xml");
}

bool hasEpubTextTag(const char* name)
{
	if (!name)
	{
		return false;
	}
	return strcmp(name, "p") == 0 || strcmp(name, "div") == 0 || strcmp(name, "section") == 0 || strcmp(name, "article") == 0 || strcmp(name, "main") == 0 || strcmp(name, "body") == 0 || strcmp(name, "header") == 0 || strcmp(name, "footer") == 0 || strcmp(name, "aside") == 0 || strcmp(name, "nav") == 0 || strcmp(name, "li") == 0 || strcmp(name, "ul") == 0 || strcmp(name, "ol") == 0 || strcmp(name, "table") == 0 || strcmp(name, "tr") == 0 || strcmp(name, "td") == 0 || strcmp(name, "th") == 0 || strcmp(name, "blockquote") == 0 || strcmp(name, "pre") == 0 || strcmp(name, "h1") == 0 || strcmp(name, "h2") == 0 || strcmp(name, "h3") == 0 || strcmp(name, "h4") == 0 || strcmp(name, "h5") == 0 || strcmp(name, "h6") == 0 || strcmp(name, "br") == 0;
}

void appendNormalizedText(const char* text, String& output)
{
	if (!text)
	{
		return;
	}

	bool lastWasSpace = output.isEmpty() ? true : isspace(static_cast<unsigned char>(output[output.length() - 1]));
	for (size_t i = 0; text[i] != '\0'; ++i)
	{
		const unsigned char ch = static_cast<unsigned char>(text[i]);
		if (isspace(ch))
		{
			if (!lastWasSpace)
			{
				output += ' ';
				lastWasSpace = true;
			}
		}
		else
		{
			output += static_cast<char>(ch);
			lastWasSpace = false;
		}
	}
}

void extractTextFromXmlNode(const tinyxml2::XMLNode* node, String& output)
{
	if (!node)
	{
		return;
	}

	if (const tinyxml2::XMLText* text = node->ToText())
	{
		appendNormalizedText(text->Value(), output);
		return;
	}

	const tinyxml2::XMLElement* element = node->ToElement();
	const bool isBlock = element && hasEpubTextTag(element->Name());
	if (isBlock && !output.isEmpty() && output[output.length() - 1] != '\n')
	{
		output += '\n';
	}

	for (const tinyxml2::XMLNode* child = node->FirstChild(); child; child = child->NextSibling())
	{
		extractTextFromXmlNode(child, output);
	}

	if (element && strcmp(element->Name(), "br") == 0)
	{
		output += '\n';
	}
	else if (isBlock && !output.isEmpty() && output[output.length() - 1] != '\n')
	{
		output += '\n';
	}
}

String xmlDocumentToText(tinyxml2::XMLDocument& document)
{
	String output;
	output.reserve(8192);
	for (const tinyxml2::XMLNode* child = document.FirstChild(); child; child = child->NextSibling())
	{
		extractTextFromXmlNode(child, output);
	}
	output.replace("\r", "");
	output.replace("\n ", "\n");
	output.replace("  ", " ");
	while (output.indexOf("\n\n\n") >= 0)
	{
		output.replace("\n\n\n", "\n\n");
	}
	return output;
}

String markupToText(const String& markup)
{
	String output;
	output.reserve(markup.length() > 32768 ? 32768 : markup.length());

	bool inTag = false;
	char tagName[16] = {0};
	uint8_t tagLen = 0;
	char tagAttrs[96] = {0};
	uint8_t attrsLen = 0;
	bool collectingTagName = false;
	bool sawTagNameChar = false;
	bool lastWasSpace = true;
	uint8_t boldDepth = 0;
	uint8_t obliqueDepth = 0;
	uint8_t headerDepth = 0;
	uint8_t attrBoldDepth = 0;
	uint8_t attrObliqueDepth = 0;
	ReaderFontStyle activeStyle = ReaderFontStyle::Normal;
	ReaderTextAlign activeAlign = ReaderTextAlign::Left;

	auto currentStyle = [&]() -> ReaderFontStyle
	{
		const bool bold = (boldDepth > 0) || (headerDepth > 0) || (attrBoldDepth > 0);
		const bool oblique = (obliqueDepth > 0) || (attrObliqueDepth > 0);
		if (bold && oblique)
		{
			return ReaderFontStyle::BoldOblique;
		}
		if (bold)
		{
			return ReaderFontStyle::Bold;
		}
		if (oblique)
		{
			return ReaderFontStyle::Oblique;
		}
		return ReaderFontStyle::Normal;
	};

	auto maybeEmitStyleMarker = [&]()
	{
		const ReaderFontStyle desired = currentStyle();
		if (desired == activeStyle)
		{
			return;
		}
		appendStyleMarker(output, desired);
		activeStyle = desired;
	};

	auto maybeEmitAlignMarker = [&](ReaderTextAlign desired)
	{
		if (desired == activeAlign)
		{
			return;
		}
		appendAlignMarker(output, desired);
		activeAlign = desired;
	};

	for (size_t i = 0; i < markup.length(); ++i)
	{
		const char ch = markup[i];

		if (!inTag && ch == '<')
		{
			inTag = true;
			tagLen = 0;
			attrsLen = 0;
			tagAttrs[0] = '\0';
			collectingTagName = true;
			sawTagNameChar = false;
			continue;
		}

		if (inTag)
		{
			if (ch == '>')
			{
				inTag = false;
				if (tagLen > 0)
				{
					const bool isCloseTag = tagName[0] == '/';
					const char* normalizedTag = isCloseTag ? tagName + 1 : tagName;
					const bool isBlockTag = strcmp(normalizedTag, "p") == 0 || strcmp(normalizedTag, "div") == 0 || strcmp(normalizedTag, "section") == 0 || strcmp(normalizedTag, "article") == 0 || strcmp(normalizedTag, "li") == 0 || strcmp(normalizedTag, "blockquote") == 0 || strcmp(normalizedTag, "h1") == 0 || strcmp(normalizedTag, "h2") == 0 || strcmp(normalizedTag, "h3") == 0 || strcmp(normalizedTag, "h4") == 0 || strcmp(normalizedTag, "h5") == 0 || strcmp(normalizedTag, "h6") == 0;
					const bool isSpanLikeTag = strcmp(normalizedTag, "span") == 0 || strcmp(normalizedTag, "a") == 0 || strcmp(normalizedTag, "p") == 0 || strcmp(normalizedTag, "div") == 0;
					const bool attrItalic = strstr(tagAttrs, "italic") || strstr(tagAttrs, "oblique") || strstr(tagAttrs, "emphasis");
					const bool attrBold = strstr(tagAttrs, "bold") || strstr(tagAttrs, "font-weight:700") || strstr(tagAttrs, "font-weight:800") || strstr(tagAttrs, "font-weight:900");
					const bool isBreakTag = strcmp(normalizedTag, "br") == 0 || strcmp(normalizedTag, "p") == 0 || strcmp(normalizedTag, "div") == 0 || strcmp(normalizedTag, "section") == 0 || strcmp(normalizedTag, "article") == 0 || strcmp(normalizedTag, "li") == 0;
					const bool isHeaderTag = strcmp(normalizedTag, "h1") == 0 || strcmp(normalizedTag, "h2") == 0 || strcmp(normalizedTag, "h3") == 0 || strcmp(normalizedTag, "h4") == 0 || strcmp(normalizedTag, "h5") == 0 || strcmp(normalizedTag, "h6") == 0;

					if (!isCloseTag && (strcmp(normalizedTag, "strong") == 0 || strcmp(normalizedTag, "b") == 0))
					{
						++boldDepth;
					}
					else if (isCloseTag && (strcmp(normalizedTag, "strong") == 0 || strcmp(normalizedTag, "b") == 0) && boldDepth > 0)
					{
						--boldDepth;
					}

					if (!isCloseTag && (strcmp(normalizedTag, "em") == 0 || strcmp(normalizedTag, "i") == 0))
					{
						++obliqueDepth;
					}
					else if (isCloseTag && (strcmp(normalizedTag, "em") == 0 || strcmp(normalizedTag, "i") == 0) && obliqueDepth > 0)
					{
						--obliqueDepth;
					}

					if (!isCloseTag && isSpanLikeTag)
					{
						if (attrBold)
						{
							++attrBoldDepth;
						}
						if (attrItalic)
						{
							++attrObliqueDepth;
						}
					}
					else if (isCloseTag && isSpanLikeTag)
					{
						if (attrBoldDepth > 0)
						{
							--attrBoldDepth;
						}
						if (attrObliqueDepth > 0)
						{
							--attrObliqueDepth;
						}
					}

					if (!isCloseTag && isHeaderTag)
					{
						++headerDepth;
					}
					else if (isCloseTag && isHeaderTag && headerDepth > 0)
					{
						--headerDepth;
					}

					if (isBreakTag || isHeaderTag)
					{
						if (!output.isEmpty() && output[output.length() - 1] != '\n')
						{
							output += '\n';
						}
						lastWasSpace = true;
					}

					if (!isCloseTag && isBlockTag)
					{
						bool hasAlign = false;
						const ReaderTextAlign resolvedAlign = resolveTagAlignment(String(tagAttrs), hasAlign);
						// Alignment is treated per block; blocks with no explicit alignment fall back to left.
						maybeEmitAlignMarker(hasAlign ? resolvedAlign : ReaderTextAlign::Left);
					}

					maybeEmitStyleMarker();
				}
				continue;
			}

			if (collectingTagName)
			{
				if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
				{
					collectingTagName = false;
					continue;
				}

				if (!sawTagNameChar && (ch == '!' || ch == '?'))
				{
					collectingTagName = false;
					continue;
				}

				if (tagLen < sizeof(tagName) - 1)
				{
					tagName[tagLen++] = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
					tagName[tagLen] = '\0';
					sawTagNameChar = true;
				}
			}
			else if (attrsLen < sizeof(tagAttrs) - 1)
			{
				tagAttrs[attrsLen++] = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
				tagAttrs[attrsLen] = '\0';
			}

			continue;
		}

		if (ch == '&')
		{
			const int semi = markup.indexOf(';', i);
			if (semi > static_cast<int>(i) && semi - static_cast<int>(i) <= 10)
			{
				const String entity = markup.substring(i + 1, semi);
				String replacement;
				if (decodeEntityToAscii(entity, replacement))
				{
					for (size_t r = 0; r < replacement.length(); ++r)
					{
						const char outChar = replacement[r];
						if (isspace(static_cast<unsigned char>(outChar)))
						{
							if (!lastWasSpace)
							{
								output += ' ';
								lastWasSpace = true;
							}
						}
						else
						{
							output += outChar;
							lastWasSpace = false;
						}
					}
				}
				else
				{
					// Keep unknown entities visible instead of silently dropping characters.
					output += '&';
					output += entity;
					output += ';';
					lastWasSpace = false;
				}

				i = static_cast<size_t>(semi);
				continue;
			}
		}

		if (isspace(static_cast<unsigned char>(ch)))
		{
			if (!lastWasSpace)
			{
				output += ' ';
				lastWasSpace = true;
			}
		}
		else
		{
			output += ch;
			lastWasSpace = false;
		}

		if ((i & 0x3FF) == 0)
		{
			yield();
		}
	}

	output.replace("\r", "");
	normalizeSmartPunctuationAscii(output);
	while (output.indexOf("\n\n\n") >= 0)
	{
		output.replace("\n\n\n", "\n\n");
	}
	if (activeStyle != ReaderFontStyle::Normal)
	{
		appendStyleMarker(output, ReaderFontStyle::Normal);
	}
	if (activeAlign != ReaderTextAlign::Left)
	{
		appendAlignMarker(output, ReaderTextAlign::Left);
	}
	return output;
}

void* epubOpenCallback(const char* filename, int32_t* size)
{
	epubFsFile = LittleFS.open(filename, "r");
	if (!epubFsFile)
	{
		return nullptr;
	}

	*size = static_cast<int32_t>(epubFsFile.size());
	return &epubFsFile;
}

void epubCloseCallback(void* pFile)
{
	ZIPFILE* pzf = static_cast<ZIPFILE*>(pFile);
	if (pzf && pzf->fHandle)
	{
		static_cast<File*>(pzf->fHandle)->close();
	}
}

int32_t epubReadCallback(void* pFile, uint8_t* buffer, int32_t length)

{
	ZIPFILE* pzf = static_cast<ZIPFILE*>(pFile);
	if (!pzf || !pzf->fHandle)
	{
		return 0;
	}
	const int32_t bytes = static_cast<File*>(pzf->fHandle)->read(buffer, length);
	yield();
	return bytes;
}

int32_t epubSeekCallback(void* pFile, int32_t position, int type)

{
	ZIPFILE* pzf = static_cast<ZIPFILE*>(pFile);
	File* file = (pzf && pzf->fHandle) ? static_cast<File*>(pzf->fHandle) : nullptr;
	if (!file)
	{
		return 0;
	}

	int32_t target = position;
	if (type == SEEK_CUR)
	{
		target += static_cast<int32_t>(file->position());
	}
	else if (type == SEEK_END)
	{
		if (target > 0)
		{
			target = 0;
		}
		target += static_cast<int32_t>(file->size());
	}

	if (target < 0)
	{
		target = 0;
	}
	else if (target > static_cast<int32_t>(file->size()))
	{
		target = static_cast<int32_t>(file->size());
	}

	if (!file->seek(static_cast<uint32_t>(target), SeekSet))
	{
		return -1;
	}
	yield();
	return target;
}

bool readCurrentZipEntryToString(UNZIP& zip, String& output, size_t maxBytes = kReaderMaxExtractBytes)
{
	if (zip.openCurrentFile() != UNZ_OK)
	{
		return false;
	}

	output = "";
	output.reserve(4096);
	uint8_t buffer[256];
	int32_t bytesRead = 0;
	size_t totalRead = 0;
	uint32_t chunkCount = 0;
	while ((bytesRead = zip.readCurrentFile(buffer, sizeof(buffer))) > 0)
	{
		totalRead += static_cast<size_t>(bytesRead);
		for (int32_t i = 0; i < bytesRead; ++i)
		{
			output += static_cast<char>(buffer[i]);
		}

		++chunkCount;
		if ((chunkCount & 0x1F) == 0)
		{
			yield();
		}

		if (totalRead >= maxBytes)
		{
			break;
		}
	}

	zip.closeCurrentFile();
	return bytesRead >= 0;

}

bool readNamedZipEntryToString(UNZIP& zip, const char* entryName, String& output, size_t maxBytes = kReaderMaxExtractBytes)

{
	if (!entryName)
	{
		return false;
	}

	if (zip.locateFile(entryName) != UNZ_OK)
	{
		String alternate = String(entryName);
		if (alternate.startsWith("/"))
		{
			alternate = alternate.substring(1);
		}
		else
		{
			alternate = "/" + alternate;
		}

		if (zip.locateFile(alternate.c_str()) != UNZ_OK)
		{
			return false;
		}
	}

	return readCurrentZipEntryToString(zip, output, maxBytes);
}

bool readNamedZipEntryToBytes(UNZIP& zip, const char* entryName, uint8_t*& output, size_t& outputSize, size_t maxBytes = kReaderMaxCoverBytes)
{
	output = nullptr;
	outputSize = 0;
	size_t expectedSize = 0;
	if (!entryName)
	{
		return false;
	}

	bool located = (zip.locateFile(entryName) == UNZ_OK);
	if (located)
	{
		unz_file_info fileInfo;
		char fileName[UNZ_MAXFILENAMEINZIP];
		if (zip.getFileInfo(&fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0) == UNZ_OK)
		{
			expectedSize = static_cast<size_t>(fileInfo.uncompressed_size);
		}
	}
	if (!located)
	{
		String wanted = String(entryName);
		if (wanted.startsWith("/"))
		{
			wanted = wanted.substring(1);
			located = (zip.locateFile(wanted.c_str()) == UNZ_OK);
			if (located)
			{
				unz_file_info fileInfo;
				char fileName[UNZ_MAXFILENAMEINZIP];
				if (zip.getFileInfo(&fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0) == UNZ_OK)
				{
					expectedSize = static_cast<size_t>(fileInfo.uncompressed_size);
				}
			}
		}
	}

	if (!located)
	{
		// Fallback: scan for a matching basename (handles archive quirks on long/special names)
		String wantedName = String(entryName);
		const int wantedSlash = wantedName.lastIndexOf('/');
		String wantedBase = wantedSlash >= 0 ? wantedName.substring(wantedSlash + 1) : wantedName;
		wantedBase.toLowerCase();

		if (zip.gotoFirstFile() == UNZ_OK)
		{
			int rc = UNZ_OK;
			while (rc == UNZ_OK)
			{
				char fileName[UNZ_MAXFILENAMEINZIP];
				unz_file_info fileInfo;
				rc = zip.getFileInfo(&fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0);
				if (rc == UNZ_OK)
				{
					String candidate = String(fileName);
					String candidateLower = candidate;
					candidateLower.toLowerCase();

					if (candidate == String(entryName))
					{
						located = true;
						expectedSize = static_cast<size_t>(fileInfo.uncompressed_size);
						break;
					}

					const int slash = candidateLower.lastIndexOf('/');
					const String candidateBase = slash >= 0 ? candidateLower.substring(slash + 1) : candidateLower;
					if (candidateBase == wantedBase)
					{
						located = true;
						expectedSize = static_cast<size_t>(fileInfo.uncompressed_size);
						break;
					}
				}

				rc = zip.gotoNextFile();
				yield();
			}
		}
	}

	if (!located)
	{
		Serial.print("[COVER] locateFile failed for: ");
		Serial.println(entryName);
		return false;
	}

	if (expectedSize > maxBytes)
	{
		Serial.println("[COVER] cover exceeds configured max size");
		return false;
	}

	size_t capacity = expectedSize > 0 ? expectedSize : 16384;
	if (capacity > maxBytes)
	{
		capacity = maxBytes;
	}

	output = static_cast<uint8_t*>(malloc(capacity));
	if (!output)
	{
		Serial.print("[COVER] malloc failed for cover buffer size=");
		Serial.println(static_cast<uint32_t>(capacity));
		return false;
	}

	if (zip.openCurrentFile() != UNZ_OK)
	{
		Serial.println("[COVER] openCurrentFile failed");
		free(output);
		output = nullptr;
		return false;
	}

	uint8_t chunk[512];
	int32_t bytesRead = 0;
	size_t totalRead = 0;
	uint32_t chunkCount = 0;
	while ((bytesRead = zip.readCurrentFile(chunk, sizeof(chunk))) > 0)
	{
		const size_t wanted = static_cast<size_t>(bytesRead);
		if (totalRead + wanted > maxBytes)
		{
			Serial.println("[COVER] cover exceeds max buffer size");
			zip.closeCurrentFile();
			free(output);
			output = nullptr;
			outputSize = 0;
			return false;
		}

		if (totalRead + wanted > capacity)
		{
			size_t newCapacity = capacity;
			while (newCapacity < totalRead + wanted && newCapacity < maxBytes)
			{
				newCapacity *= 2;
				if (newCapacity > maxBytes)
				{
					newCapacity = maxBytes;
				}
			}

			if (newCapacity < totalRead + wanted)
			{
				zip.closeCurrentFile();
				free(output);
				output = nullptr;
				outputSize = 0;
				return false;
			}

			uint8_t* resized = static_cast<uint8_t*>(realloc(output, newCapacity));
			if (!resized)
			{
				Serial.print("[COVER] realloc failed newCapacity=");
				Serial.println(static_cast<uint32_t>(newCapacity));
				zip.closeCurrentFile();
				free(output);
				output = nullptr;
				outputSize = 0;
				return false;
			}

			output = resized;
			capacity = newCapacity;
		}

		memcpy(output + totalRead, chunk, wanted);
		totalRead += wanted;
		++chunkCount;
		if ((chunkCount & 0x0F) == 0)
		{
			yield();
		}
	}

	zip.closeCurrentFile();
	if (bytesRead < 0 || totalRead == 0)
	{
		Serial.print("[COVER] readCurrentFile failed bytesRead=");
		Serial.println(bytesRead);
		free(output);
		output = nullptr;
		outputSize = 0;
		return false;
	}

	outputSize = totalRead;
	return true;
}

String parseOpfTitle(const String& opfText)
{
	tinyxml2::XMLDocument document;
	if (document.Parse(opfText.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return "";
	}

	const tinyxml2::XMLElement* package = document.FirstChildElement("package");
	if (!package)
	{
		return "";
	}

	const tinyxml2::XMLElement* metadata = package->FirstChildElement("metadata");
	if (!metadata)
	{
		return "";
	}

	const tinyxml2::XMLElement* title = metadata->FirstChildElement("dc:title");
	if (!title || !title->GetText())
	{
		return "";
	}

	return title->GetText();
}

String parseContainerRootfile(const String& containerXml)
{
	tinyxml2::XMLDocument document;
	if (document.Parse(containerXml.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return "";
	}

	const tinyxml2::XMLElement* container = document.FirstChildElement("container");
	if (!container)
	{
		return "";
	}

	const tinyxml2::XMLElement* rootfiles = container->FirstChildElement("rootfiles");
	if (!rootfiles)
	{
		return "";
	}

	const tinyxml2::XMLElement* rootfile = rootfiles->FirstChildElement("rootfile");
	if (!rootfile)
	{
		return "";
	}

	const char* fullPath = rootfile->Attribute("full-path");
	return fullPath ? String(fullPath) : String();
}

std::vector<String> parseOpfSpineChapters(const String& opfText)
{
	tinyxml2::XMLDocument document;
	if (document.Parse(opfText.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return {};
	}

	const tinyxml2::XMLElement* package = document.FirstChildElement("package");
	if (!package)
	{
		return {};
	}

	std::vector<String> manifestIds;
	std::vector<String> manifestHrefs;
	const tinyxml2::XMLElement* manifest = package->FirstChildElement("manifest");
	if (!manifest)
	{
		return {};
	}

	for (const tinyxml2::XMLElement* item = manifest->FirstChildElement("item"); item; item = item->NextSiblingElement("item"))
	{
		const char* id = item->Attribute("id");
		const char* href = item->Attribute("href");
		if (id && href)
		{
			manifestIds.push_back(String(id));
			manifestHrefs.push_back(String(href));
		}
	}

	const tinyxml2::XMLElement* spine = package->FirstChildElement("spine");
	if (!spine)
	{
		return {};
	}

	std::vector<String> chapters;
	for (const tinyxml2::XMLElement* itemref = spine->FirstChildElement("itemref"); itemref; itemref = itemref->NextSiblingElement("itemref"))
	{
		const char* idref = itemref->Attribute("idref");
		if (!idref)
		{
			continue;
		}

		for (size_t i = 0; i < manifestIds.size(); ++i)
		{
			if (manifestIds[i] == idref)
			{
				chapters.push_back(manifestHrefs[i]);
				break;
			}
		}
	}

	return chapters;
}

bool stringHasToken(const char* value, const char* token)
{
	if (!value || !token || token[0] == '\0')
	{
		return false;
	}

	String haystack = String(value);
	haystack.toLowerCase();
	String needle = String(token);
	needle.toLowerCase();

	if (haystack == needle)
	{
		return true;
	}

	if (haystack.startsWith(needle + " ") || haystack.endsWith(" " + needle) || haystack.indexOf(" " + needle + " ") >= 0)
	{
		return true;
	}

	return false;
}

void appendUniquePath(std::vector<String>& paths, const String& path)
{
	for (const String& existing : paths)
	{
		if (existing == path)
		{
			return;
		}
	}
	paths.push_back(path);
}

void collectNavHrefsRecursive(const tinyxml2::XMLElement* element, bool activeTocNav, std::vector<String>& hrefs)
{
	for (const tinyxml2::XMLElement* current = element; current; current = current->NextSiblingElement())
	{
		bool childActive = activeTocNav;
		const char* name = current->Name();
		if (name && strcmp(name, "nav") == 0)
		{
			const char* id = current->Attribute("id");
			const char* role = current->Attribute("role");
			const char* type = current->Attribute("epub:type");
			if ((id && String(id).indexOf("toc") >= 0) || stringHasToken(role, "doc-toc") || stringHasToken(type, "toc"))
			{
				childActive = true;
			}
		}

		if (childActive && name && strcmp(name, "a") == 0)
		{
			const char* href = current->Attribute("href");
			if (href && href[0] != '\0')
			{
				appendUniquePath(hrefs, String(href));
			}
		}

		if (const tinyxml2::XMLElement* child = current->FirstChildElement())
		{
			collectNavHrefsRecursive(child, childActive, hrefs);
		}
	}
}

String collectElementText(const tinyxml2::XMLElement* element)
{
	if (!element)
	{
		return "";
	}

	String text;
	for (const tinyxml2::XMLNode* node = element->FirstChild(); node; node = node->NextSibling())
	{
		if (const tinyxml2::XMLText* t = node->ToText())
		{
			text += t->Value();
		}
		else if (const tinyxml2::XMLElement* childEl = node->ToElement())
		{
			text += collectElementText(childEl);
		}
	}

	text.replace("\r", " ");
	text.replace("\n", " ");
	while (text.indexOf("  ") >= 0)
	{
		text.replace("  ", " ");
	}
	text.trim();
	return text;
}

void collectNavEntriesRecursive(const tinyxml2::XMLElement* element, bool activeTocNav, std::vector<ReaderNavEntry>& entries)
{
	for (const tinyxml2::XMLElement* current = element; current; current = current->NextSiblingElement())
	{
		bool childActive = activeTocNav;
		const char* name = current->Name();
		if (name && strcmp(name, "nav") == 0)
		{
			const char* id = current->Attribute("id");
			const char* role = current->Attribute("role");
			const char* type = current->Attribute("epub:type");
			if ((id && String(id).indexOf("toc") >= 0) || stringHasToken(role, "doc-toc") || stringHasToken(type, "toc"))
			{
				childActive = true;
			}
		}

		if (childActive && name && strcmp(name, "a") == 0)
		{
			const char* href = current->Attribute("href");
			if (href && href[0] != '\0')
			{
				ReaderNavEntry entry;
				entry.href = String(href);
				entry.title = collectElementText(current);
				entries.push_back(entry);
			}
		}

		if (const tinyxml2::XMLElement* child = current->FirstChildElement())
		{
			collectNavEntriesRecursive(child, childActive, entries);
		}
	}
}

std::vector<ReaderNavEntry> parseOpfNavEntries(const String& navText)
{
	tinyxml2::XMLDocument document;
	if (document.Parse(navText.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return {};
	}

	std::vector<ReaderNavEntry> entries;
	collectNavEntriesRecursive(document.RootElement(), false, entries);
	return entries;
}

void collectNcxEntriesRecursive(const tinyxml2::XMLElement* element, std::vector<ReaderNavEntry>& entries)
{
	for (const tinyxml2::XMLElement* current = element; current; current = current->NextSiblingElement())
	{
		const char* name = current->Name();
		if (name && strcmp(name, "navPoint") == 0)
		{
			ReaderNavEntry entry;
			const tinyxml2::XMLElement* navLabel = current->FirstChildElement("navLabel");
			if (navLabel)
			{
				const tinyxml2::XMLElement* textEl = navLabel->FirstChildElement("text");
				if (textEl && textEl->GetText())
				{
					entry.title = String(textEl->GetText());
					entry.title.trim();
				}
			}

			const tinyxml2::XMLElement* content = current->FirstChildElement("content");
			if (content)
			{
				const char* src = content->Attribute("src");
				if (src && src[0] != '\0')
				{
					entry.href = String(src);
				}
			}

			if (!entry.href.isEmpty())
			{
				entries.push_back(entry);
			}
		}

		if (const tinyxml2::XMLElement* child = current->FirstChildElement())
		{
			collectNcxEntriesRecursive(child, entries);
		}
	}
}

std::vector<ReaderNavEntry> parseNcxEntries(const String& ncxText)
{
	tinyxml2::XMLDocument document;
	if (document.Parse(ncxText.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return {};
	}

	std::vector<ReaderNavEntry> entries;
	collectNcxEntriesRecursive(document.RootElement(), entries);
	return entries;
}

std::vector<String> parseOpfNavHrefs(const String& navText)
{
	tinyxml2::XMLDocument document;
	if (document.Parse(navText.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return {};
	}

	std::vector<String> hrefs;
	collectNavHrefsRecursive(document.RootElement(), false, hrefs);
	return hrefs;
}

String parseOpfNavDocumentHref(const String& opfText)
{
	tinyxml2::XMLDocument document;
	if (document.Parse(opfText.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return "";
	}

	const tinyxml2::XMLElement* package = document.FirstChildElement("package");
	if (!package)
	{
		return "";
	}

	const tinyxml2::XMLElement* manifest = package->FirstChildElement("manifest");
	if (!manifest)
	{
		return "";
	}

	for (const tinyxml2::XMLElement* item = manifest->FirstChildElement("item"); item; item = item->NextSiblingElement("item"))
	{
		const char* properties = item->Attribute("properties");
		const char* href = item->Attribute("href");
		const char* id = item->Attribute("id");
		if (href && ((properties && stringHasToken(properties, "nav")) || (id && String(id).indexOf("nav") >= 0) || String(href).endsWith("nav.xhtml") || String(href).endsWith("nav.html")))
		{
			return String(href);
		}
	}

	return "";
}

String parseOpfNcxDocumentHref(const String& opfText)
{
	tinyxml2::XMLDocument document;
	if (document.Parse(opfText.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return "";
	}

	const tinyxml2::XMLElement* package = document.FirstChildElement("package");
	if (!package)
	{
		return "";
	}

	String spineTocId;
	const tinyxml2::XMLElement* spine = package->FirstChildElement("spine");
	if (spine)
	{
		const char* tocId = spine->Attribute("toc");
		if (tocId && tocId[0] != '\0')
		{
			spineTocId = String(tocId);
		}
	}

	const tinyxml2::XMLElement* manifest = package->FirstChildElement("manifest");
	if (!manifest)
	{
		return "";
	}

	for (const tinyxml2::XMLElement* item = manifest->FirstChildElement("item"); item; item = item->NextSiblingElement("item"))
	{
		const char* href = item->Attribute("href");
		if (!href)
		{
			continue;
		}

		const char* mediaType = item->Attribute("media-type");
		const char* id = item->Attribute("id");
		const String hrefValue = String(href);
		if ((!spineTocId.isEmpty() && id && spineTocId == id) ||
			(mediaType && String(mediaType) == "application/x-dtbncx+xml") ||
			hrefValue.endsWith(".ncx"))
		{
			return hrefValue;
		}
	}

	return "";
}

String parseOpfCoverHref(const String& opfText)
{
	tinyxml2::XMLDocument document;
	if (document.Parse(opfText.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return "";
	}

	const tinyxml2::XMLElement* package = document.FirstChildElement("package");
	if (!package)
	{
		return "";
	}

	const tinyxml2::XMLElement* manifest = package->FirstChildElement("manifest");
	if (!manifest)
	{
		return "";
	}

	auto hrefById = [&](const String& wantedId) -> String
	{
		for (const tinyxml2::XMLElement* item = manifest->FirstChildElement("item"); item; item = item->NextSiblingElement("item"))
		{
			const char* id = item->Attribute("id");
			const char* properties = item->Attribute("properties");
			const char* href = item->Attribute("href");
			if (!href)
			{
				continue;
			}

			if ((id && wantedId == id) || (properties && wantedId == properties))
			{
				return String(href);
			}
		}
		return "";
	};

	const tinyxml2::XMLElement* metadata = package->FirstChildElement("metadata");
	if (metadata)
	{
		for (const tinyxml2::XMLElement* meta = metadata->FirstChildElement("meta"); meta; meta = meta->NextSiblingElement("meta"))
		{
			const char* name = meta->Attribute("name");
			const char* content = meta->Attribute("content");
			if (name && content && strcmp(name, "cover") == 0)
			{
				const String href = hrefById(String(content));
				if (!href.isEmpty())
				{
					return href;
				}
			}
		}
	}

	for (const tinyxml2::XMLElement* item = manifest->FirstChildElement("item"); item; item = item->NextSiblingElement("item"))
	{
		const char* id = item->Attribute("id");
		const char* properties = item->Attribute("properties");
		const char* href = item->Attribute("href");
		if (!href)
		{
			continue;
		}

		if ((id && (strcmp(id, "cover-image") == 0 || strcmp(id, "cover") == 0)) ||
			(properties && (stringHasToken(properties, "cover-image") || stringHasToken(properties, "cover"))))
		{
			return String(href);
		}
	}

	return "";
}

std::vector<String> parseOpfStylesheetHrefs(const String& opfText)
{
	tinyxml2::XMLDocument document;
	if (document.Parse(opfText.c_str()) != tinyxml2::XML_SUCCESS)
	{
		return {};
	}

	const tinyxml2::XMLElement* package = document.FirstChildElement("package");
	if (!package)
	{
		return {};
	}

	const tinyxml2::XMLElement* manifest = package->FirstChildElement("manifest");
	if (!manifest)
	{
		return {};
	}

	std::vector<String> stylesheets;
	for (const tinyxml2::XMLElement* item = manifest->FirstChildElement("item"); item; item = item->NextSiblingElement("item"))
	{
		const char* href = item->Attribute("href");
		const char* mediaType = item->Attribute("media-type");
		if (!href)
		{
			continue;
		}

		const String hrefValue = String(href);
		if ((mediaType && String(mediaType) == "text/css") || hrefValue.endsWith(".css"))
		{
			stylesheets.push_back(hrefValue);
		}
	}

	return stylesheets;
}

bool collectEpubTextFromPath(const String& epubPath, String& bookTitle, String& chapterText)
{
	Serial.println("[EPUB] open start");
	epubZip = new (std::nothrow) UNZIP();
	if (!epubZip)
	{
		Serial.println("[EPUB] alloc failed");
		return false;
	}

	if (epubZip->openZIP(epubPath.c_str(), epubOpenCallback, epubCloseCallback, epubReadCallback, epubSeekCallback) != 0)
	{
		Serial.println("[EPUB] zip open failed");
		delete epubZip;
		epubZip = nullptr;
		return false;
	}
	Serial.println("[EPUB] zip opened");

	auto cleanupZip = []()
	{
		if (epubZip)
		{
			epubZip->closeZIP();
			delete epubZip;
			epubZip = nullptr;
		}
	};

	bool success = false;
	String containerXml;
	if (readNamedZipEntryToString(*epubZip, "META-INF/container.xml", containerXml))
	{
		Serial.println("[EPUB] container read ok");
		const String rootfilePath = parseContainerRootfile(containerXml);
		if (rootfilePath.length() > 0)
		{
			String opfText;
			if (readNamedZipEntryToString(*epubZip, rootfilePath.c_str(), opfText))
			{
				Serial.println("[EPUB] opf read ok");
				const String parsedTitle = parseOpfTitle(opfText);
				if (parsedTitle.length() > 0)
				{
					bookTitle = parsedTitle;
				}

				std::vector<String> chapterRelativePaths = parseOpfSpineChapters(opfText);
				const String navDocHref = parseOpfNavDocumentHref(opfText);
				if (navDocHref.length() > 0)
				{
					String navDocText;
					const String navDocPath = resolveRelativePath(rootfilePath, navDocHref);
					if (readNamedZipEntryToString(*epubZip, navDocPath.c_str(), navDocText))
					{
						const std::vector<String> navChapterPaths = parseOpfNavHrefs(navDocText);
						if (!navChapterPaths.empty())
						{
							std::vector<String> mergedChapterPaths;
							mergedChapterPaths.reserve(navChapterPaths.size() + chapterRelativePaths.size());
							for (const String& navPath : navChapterPaths)
							{
								appendUniquePath(mergedChapterPaths, navPath);
							}
							for (const String& spinePath : chapterRelativePaths)
							{
								appendUniquePath(mergedChapterPaths, spinePath);
							}
							chapterRelativePaths = mergedChapterPaths;
						}
					}
				}

				if (!chapterRelativePaths.empty())
				{
					for (size_t i = 0; i < chapterRelativePaths.size(); ++i)
					{
						const String chapterPath = resolveRelativePath(rootfilePath, chapterRelativePaths[i]);
						String chapterXml;
						if (!readNamedZipEntryToString(*epubZip, chapterPath.c_str(), chapterXml, 40000))
						{
							yield();
							continue;
						}

						const String chapterPart = markupToText(chapterXml);
						if (chapterPart.length() > 30)
						{
							if (!chapterText.isEmpty())
							{
								chapterText += "\n\n";
							}
							chapterText += chapterPart;
						}

						if (chapterText.length() >= kReaderMaxBookTextChars)
						{
							break;
						}

						yield();
					}

					success = chapterText.length() > 0;
				}
			}
		}
	}

	if (!success)
	{
		Serial.println("[EPUB] fallback scan");
		epubZip->gotoFirstFile();
		int rc = UNZ_OK;
		while (rc == UNZ_OK)
		{
			char fileName[UNZ_MAXFILENAMEINZIP];
			unz_file_info fileInfo;
			rc = epubZip->getFileInfo(&fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0);
			if (rc == UNZ_OK && hasTextExtension(String(fileName)))
			{
				String fallbackXml;
				if (readCurrentZipEntryToString(*epubZip, fallbackXml))
				{
							const String fallbackText = markupToText(fallbackXml);
							if (fallbackText.length() > 0)
					{
								chapterText = fallbackText;
								if (!chapterText.isEmpty())
						{
							if (bookTitle.isEmpty())
							{
								bookTitle = String(fileName);
							}
							success = true;
							break;
						}
					}
				}
			}
			rc = epubZip->gotoNextFile();
			yield();
		}
	}

	cleanupZip();
	return success;
}

void closeReaderChapterStream();
bool countReaderChapterPages(uint16_t chapterIndex, uint16_t& outPageCount);
bool buildReaderBookPageIndex();

void saveCurrentReaderProgress()
{
	if (readerBookPath.isEmpty())
	{
		return;
	}

	saveReaderBookmark(readerBookPath, readerChapterIndex, readerPageIndex, readerShowingCover);
	readerResumeChapterIndex = readerChapterIndex;
	readerResumePageIndex = readerPageIndex;
	readerResumeShowingCover = readerShowingCover;
}

void clearReaderState()
{
	saveCurrentReaderProgress();
	flushReaderBookmarksToStorage(true);
	readerBookPath = "";
	readerBookTitle = "";
	resetReaderPageBuilderState();
	resetReaderMarkupStreamState();
	readerChapterPaths.clear();
	readerChapterTitles.clear();
	readerChapterIndex = 0;
	readerResumeChapterIndex = 0;
	readerResumePageIndex = 0;
	readerResumeShowingCover = false;
	readerCoverReturnPageIndex = 0;
	readerJumpTargetChapterIndex = 0;
	readerTotalPagesLoaded = 0;
	for (uint16_t i = 0; i < kMaxBooks; ++i) { readerChapterPageCounts[i] = 0; }
	readerCoverPath = "";
	readerHasCover = false;
	readerShowingCover = false;
	readerCssAlignCenterClasses.clear();
	readerCssAlignRightClasses.clear();
	closeReaderChapterStream();
}

bool openNamedZipEntryForStreaming(UNZIP& zip, const char* entryName)
{
	if (!entryName)
	{
		return false;
	}

	if (zip.locateFile(entryName) != UNZ_OK)
	{
		String alternate = String(entryName);
		if (alternate.startsWith("/"))
		{
			alternate = alternate.substring(1);
		}
		else
		{
			alternate = "/" + alternate;
		}

		if (zip.locateFile(alternate.c_str()) != UNZ_OK)
		{
			return false;
		}
	}

	return zip.openCurrentFile() == UNZ_OK;
}

void closeReaderChapterStream()
{
	if (!epubZip)
	{
		readerChapterFileOpen = false;
		readerChapterInputEnded = true;
		return;
	}

	if (readerChapterFileOpen)
	{
		epubZip->closeCurrentFile();
		readerChapterFileOpen = false;
	}

	epubZip->closeZIP();
	delete epubZip;
	epubZip = nullptr;
	readerChapterInputEnded = true;
}

void buildReaderPagesFromText(const String& text);
bool ensureReaderPageBuilt(int32_t pageIndex);

bool collectEpubChapterPaths(const String& epubPath, String& bookTitle, std::vector<String>& chapterPaths, std::vector<String>& chapterTitles)
{
	chapterPaths.clear();
	chapterTitles.clear();
	readerCssAlignCenterClasses.clear();
	readerCssAlignRightClasses.clear();
	epubZip = new (std::nothrow) UNZIP();
	if (!epubZip)
	{
		return false;
	}

	if (epubZip->openZIP(epubPath.c_str(), epubOpenCallback, epubCloseCallback, epubReadCallback, epubSeekCallback) != 0)
	{
		delete epubZip;
		epubZip = nullptr;
		return false;
	}

	auto cleanupZip = []()
	{
		if (epubZip)
		{
			epubZip->closeZIP();
			delete epubZip;
			epubZip = nullptr;
		}
	};

	String containerXml;
	if (!readNamedZipEntryToString(*epubZip, "META-INF/container.xml", containerXml))
	{
		cleanupZip();
		return false;
	}

	const String rootfilePath = parseContainerRootfile(containerXml);
	if (rootfilePath.isEmpty())
	{
		cleanupZip();
		return false;
	}

	String opfText;
	if (!readNamedZipEntryToString(*epubZip, rootfilePath.c_str(), opfText))
	{
		cleanupZip();
		return false;
	}

	const std::vector<String> stylesheetHrefs = parseOpfStylesheetHrefs(opfText);
	for (const String& stylesheetHref : stylesheetHrefs)
	{
		String cssText;
		const String stylesheetPath = resolveRelativePath(rootfilePath, stylesheetHref);
		if (readNamedZipEntryToString(*epubZip, stylesheetPath.c_str(), cssText, 60000))
		{
			collectCssAlignmentClasses(cssText);
		}
		yield();
	}

	const String coverRelativePath = parseOpfCoverHref(opfText);
	if (!coverRelativePath.isEmpty())
	{
		readerCoverPath = resolveRelativePath(rootfilePath, coverRelativePath);
		readerHasCover = !readerCoverPath.isEmpty();
	}
	else
	{
		readerCoverPath = "";
		readerHasCover = false;
	}

	const String parsedTitle = parseOpfTitle(opfText);
	if (!parsedTitle.isEmpty())
	{
		bookTitle = parsedTitle;
	}

	std::vector<String> chapterRelativePaths = parseOpfSpineChapters(opfText);
	const String navDocHref = parseOpfNavDocumentHref(opfText);
	std::vector<ReaderNavEntry> navEntries;
	if (navDocHref.length() > 0)
	{
		String navDocText;
		const String navDocPath = resolveRelativePath(rootfilePath, navDocHref);
		if (readNamedZipEntryToString(*epubZip, navDocPath.c_str(), navDocText))
		{
			navEntries = parseOpfNavEntries(navDocText);
			const std::vector<String> navChapterPaths = parseOpfNavHrefs(navDocText);
			if (!navChapterPaths.empty())
			{
				std::vector<String> mergedChapterPaths;
				mergedChapterPaths.reserve(navChapterPaths.size() + chapterRelativePaths.size());
				for (const String& navPath : navChapterPaths)
				{
					appendUniquePath(mergedChapterPaths, navPath);
				}
				for (const String& spinePath : chapterRelativePaths)
				{
					appendUniquePath(mergedChapterPaths, spinePath);
				}
				chapterRelativePaths = mergedChapterPaths;
			}
		}
	}

	if (navEntries.empty())
	{
		const String ncxHref = parseOpfNcxDocumentHref(opfText);
		if (!ncxHref.isEmpty())
		{
			String ncxText;
			const String ncxPath = resolveRelativePath(rootfilePath, ncxHref);
			if (readNamedZipEntryToString(*epubZip, ncxPath.c_str(), ncxText, 200000))
			{
				navEntries = parseNcxEntries(ncxText);
			}
		}
	}

	std::vector<String> mergedRelativePaths;
	std::vector<String> mergedRelativeTitles;
	auto appendMerged = [&](const String& relPath, const String& title)
	{
		for (size_t i = 0; i < mergedRelativePaths.size(); ++i)
		{
			if (mergedRelativePaths[i] == relPath)
			{
				if (mergedRelativeTitles[i].isEmpty() && !title.isEmpty())
				{
					mergedRelativeTitles[i] = title;
				}
				return;
			}
		}
		mergedRelativePaths.push_back(relPath);
		mergedRelativeTitles.push_back(title);
	};

	for (const ReaderNavEntry& entry : navEntries)
	{
		appendMerged(entry.href, entry.title);
	}
	for (const String& relPath : chapterRelativePaths)
	{
		appendMerged(relPath, "");
	}

	auto appendResolved = [&](const String& resolvedPath, const String& title)
	{
		for (size_t i = 0; i < chapterPaths.size(); ++i)
		{
			if (chapterPaths[i] == resolvedPath)
			{
				if (chapterTitles[i].isEmpty() && !title.isEmpty())
				{
					chapterTitles[i] = title;
				}
				return;
			}
		}
		chapterPaths.push_back(resolvedPath);
		chapterTitles.push_back(title);
	};

	for (size_t i = 0; i < mergedRelativePaths.size(); ++i)
	{
		const String resolvedPath = resolveRelativePath(rootfilePath, mergedRelativePaths[i]);
		if (resolvedPath.length() > 0)
		{
			appendResolved(resolvedPath, mergedRelativeTitles[i]);
		}
	}

	cleanupZip();
	return !chapterPaths.empty();
}

bool countReaderChapterPages(uint16_t chapterIndex, uint16_t& outPageCount)
{
	outPageCount = 0;
	if (chapterIndex >= readerChapterPaths.size())
	{
		return false;
	}

	closeReaderChapterStream();
	resetReaderPageBuilderState();
	resetReaderMarkupStreamState();
	readerChapterInputEnded = false;
	readerChapterFileOpen = false;

	epubZip = new (std::nothrow) UNZIP();
	if (!epubZip)
	{
		return false;
	}

	if (epubZip->openZIP(readerBookPath.c_str(), epubOpenCallback, epubCloseCallback, epubReadCallback, epubSeekCallback) != 0)
	{
		delete epubZip;
		epubZip = nullptr;
		return false;
	}

	if (!openNamedZipEntryForStreaming(*epubZip, readerChapterPaths[chapterIndex].c_str()))
	{
		closeReaderChapterStream();
		return false;
	}
	readerChapterFileOpen = true;

	while (!readerChapterParseComplete)
	{
		if (appendNextReaderPage())
		{
			if (!readerPages.empty())
			{
				outPageCount = static_cast<uint16_t>(outPageCount + readerPages.size());
				readerPages.clear();
			}
		}
	}

	if (!readerPages.empty())
	{
		outPageCount = static_cast<uint16_t>(outPageCount + readerPages.size());
		readerPages.clear();
	}

	if (outPageCount == 0)
	{
		outPageCount = 1;
	}

	closeReaderChapterStream();
	resetReaderPageBuilderState();
	resetReaderMarkupStreamState();
	return true;
}

bool buildReaderBookPageIndex()
{
	readerTotalPagesLoaded = 0;
	for (uint16_t i = 0; i < kMaxBooks; ++i)
	{
		readerChapterPageCounts[i] = 0;
	}

	showStatusOnDisplay("Indexing book", "Please wait...");
	Serial.println("Indexing full book pages...");

	for (uint16_t i = 0; i < readerChapterPaths.size() && i < kMaxBooks; ++i)
	{
		uint16_t chapterPages = 0;
		if (!countReaderChapterPages(i, chapterPages))
		{
			return false;
		}
		readerChapterPageCounts[i] = chapterPages;
		readerTotalPagesLoaded += chapterPages;
		if ((i & 0x03) == 0)
		{
			yield();
		}
		yield();
	}

	Serial.print("Index complete. Total pages: ");
	Serial.println(readerTotalPagesLoaded);

	return true;
}

uint8_t pickCoverScaleDiv(int srcW, int srcH, int maxW, int maxH)
{
	uint8_t scaleDiv = 1;
	while (scaleDiv < 8 && ((srcW / scaleDiv) > maxW || (srcH / scaleDiv) > maxH))
	{
		scaleDiv <<= 1;
	}
	return scaleDiv;
}

void computeAspectFitRect(int srcW, int srcH, int maxW, int maxH, int& outX, int& outY, int& outW, int& outH)
{
	if (srcW <= 0 || srcH <= 0 || maxW <= 0 || maxH <= 0)
	{
		outX = 0;
		outY = 0;
		outW = 0;
		outH = 0;
		return;
	}

	if (static_cast<int64_t>(maxW) * srcH <= static_cast<int64_t>(maxH) * srcW)
	{
		outW = maxW;
		outH = static_cast<int>((static_cast<int64_t>(srcH) * outW) / srcW);
	}
	else
	{
		outH = maxH;
		outW = static_cast<int>((static_cast<int64_t>(srcW) * outH) / srcH);
	}

	if (outW < 1) outW = 1;
	if (outH < 1) outH = 1;
	outX = (maxW - outW) / 2;
	outY = (maxH - outH) / 2;
}

uint8_t pickJpegDecodeDivForTarget(int srcW, int srcH, int dstW, int dstH)
{
	uint8_t div = 1;
	while (div < 8)
	{
		const uint8_t next = static_cast<uint8_t>(div << 1);
		if ((srcW / next) < dstW || (srcH / next) < dstH)
		{
			break;
		}
		div = next;
	}
	return div;
}

uint16_t rgb565ToEpdColor(uint16_t rgb)
{
	const uint16_t r = (rgb >> 11) & 0x1F;
	const uint16_t g = (rgb >> 5) & 0x3F;
	const uint16_t b = rgb & 0x1F;
	const uint16_t gray = static_cast<uint16_t>(((r * 212U) + (g * 413U) + (b * 74U)) >> 9);
	return gray < 128 ? GxEPD_BLACK : GxEPD_WHITE;
}

int drawReaderCoverJpegBlock(JPEGDRAW* pDraw)
{
	ReaderCoverDrawContext* ctx = static_cast<ReaderCoverDrawContext*>(pDraw->pUser);
	if (!ctx || ctx->srcW == 0 || ctx->srcH == 0 || ctx->dstW == 0 || ctx->dstH == 0)
	{
		return 0;
	}

	const bool isGray8 = (pDraw->iBpp == 8);
	const uint8_t* grayPixels = isGray8 ? reinterpret_cast<const uint8_t*>(pDraw->pPixels) : nullptr;
	for (int y = 0; y < pDraw->iHeight; ++y)
	{
		const int srcY = pDraw->y + y;
		const int outY = ctx->dstY + static_cast<int>((static_cast<int64_t>(srcY) * ctx->dstH) / ctx->srcH);
		if (outY < ctx->dstY || outY >= ctx->dstY + ctx->dstH)
		{
			continue;
		}

		for (int x = 0; x < pDraw->iWidth; ++x)
		{
			const int idx = y * pDraw->iWidth + x;
			const int srcX = pDraw->x + x;
			const int outX = ctx->dstX + static_cast<int>((static_cast<int64_t>(srcX) * ctx->dstW) / ctx->srcW);
			if (outX < ctx->dstX || outX >= ctx->dstX + ctx->dstW)
			{
				continue;
			}

			if (isGray8)
			{
				display.drawPixel(outX, outY, grayPixels[idx] < 128 ? GxEPD_BLACK : GxEPD_WHITE);
			}
			else
			{
				display.drawPixel(outX, outY, rgb565ToEpdColor(pDraw->pPixels[idx]));
			}
		}
	}
	return 1;
}

int drawReaderCoverPngLine(PNGDRAW* pDraw)
{
	ReaderCoverDrawContext* ctx = static_cast<ReaderCoverDrawContext*>(pDraw->pUser);
	if (!ctx || ctx->scaleDiv == 0 || !ctx->pngDecoder)
	{
		return 0;
	}

	if ((pDraw->y % ctx->scaleDiv) != 0)
	{
		return 1;
	}

	if (ctx->pngLineCapacity < static_cast<size_t>(pDraw->iWidth))
	{
		if (ctx->pngLineBuffer)
		{
			free(ctx->pngLineBuffer);
			ctx->pngLineBuffer = nullptr;
			ctx->pngLineCapacity = 0;
		}

		ctx->pngLineBuffer = static_cast<uint16_t*>(malloc(static_cast<size_t>(pDraw->iWidth) * sizeof(uint16_t)));
		if (!ctx->pngLineBuffer)
		{
			return 0;
		}
		ctx->pngLineCapacity = static_cast<size_t>(pDraw->iWidth);
	}

	ctx->pngDecoder->getLineAsRGB565(pDraw, ctx->pngLineBuffer, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);
	const int outY = ctx->dstY + static_cast<int>((static_cast<int64_t>(pDraw->y) * ctx->dstH) / ctx->srcH);
	if (outY < ctx->dstY || outY >= ctx->dstY + ctx->dstH)
	{
		return 1;
	}

	for (int x = 0; x < pDraw->iWidth; x += ctx->scaleDiv)
	{
		const int outX = ctx->dstX + static_cast<int>((static_cast<int64_t>(x) * ctx->dstW) / ctx->srcW);
		if (outX < ctx->dstX || outX >= ctx->dstX + ctx->dstW)
		{
			continue;
		}
		display.drawPixel(outX, outY, rgb565ToEpdColor(ctx->pngLineBuffer[static_cast<size_t>(x)]));
	}

	return 1;
}

bool drawCoverImageOnDisplay(const char* imagePath, const String& coverPath)
{
	if (!imagePath || *imagePath == '\0')
	{
		return false;
	}

	String lowerPath = String(coverPath);
	lowerPath.toLowerCase();
	const bool isJpeg = lowerPath.endsWith(".jpg") || lowerPath.endsWith(".jpeg");
	const bool isPng = lowerPath.endsWith(".png");
	if (!isJpeg && !isPng)
	{
		Serial.print("[COVER] unsupported extension: ");
		Serial.println(coverPath);
		return false;
	}

	if (isJpeg)
	{
		JPEGDEC* jpeg = new (std::nothrow) JPEGDEC();
		if (!jpeg)
		{
			return false;
		}

		if (!jpeg->open(imagePath, openLittleFSFileForJpeg, closeLittleFSFileForJpeg, readLittleFSFileForJpeg, seekLittleFSFileForJpeg, drawReaderCoverJpegBlock))
		{
			Serial.println("[COVER] JPEG open failed");
			delete jpeg;
			return false;
		}

		jpeg->setPixelType(EIGHT_BIT_GRAYSCALE);

		const int imgW = jpeg->getWidth();
		const int imgH = jpeg->getHeight();

		int dstX = 0;
		int dstY = 0;
		int dstW = 0;
		int dstH = 0;
		computeAspectFitRect(imgW, imgH, display.width(), display.height(), dstX, dstY, dstW, dstH);

		const uint8_t scaleDiv = pickJpegDecodeDivForTarget(imgW, imgH, dstW, dstH);
		int decodeOptions = 0;
		if (scaleDiv == 2) decodeOptions = JPEG_SCALE_HALF;
		else if (scaleDiv == 4) decodeOptions = JPEG_SCALE_QUARTER;
		else if (scaleDiv == 8) decodeOptions = JPEG_SCALE_EIGHTH;

		ReaderCoverDrawContext context;
		context.srcW = static_cast<uint16_t>(imgW / scaleDiv);
		context.srcH = static_cast<uint16_t>(imgH / scaleDiv);
		context.dstX = static_cast<int16_t>(dstX);
		context.dstY = static_cast<int16_t>(dstY);
		context.dstW = static_cast<uint16_t>(dstW);
		context.dstH = static_cast<uint16_t>(dstH);
		jpeg->setUserPointer(&context);

		const bool decodeOk = jpeg->decode(0, 0, decodeOptions) != 0;
		if (!decodeOk)
		{
			Serial.print("[COVER] JPEG decode failed err=");
			Serial.println(jpeg->getLastError());
		}
		jpeg->close();
		delete jpeg;
		return decodeOk;
	}

	PNG* png = new (std::nothrow) PNG();
	if (!png)
	{
		return false;
	}

	ReaderCoverDrawContext context;
	context.pngDecoder = png;
	if (png->open(imagePath, openLittleFSFileForPng, closeLittleFSFileForPng, readLittleFSFileForPng, seekLittleFSFileForPng, drawReaderCoverPngLine) != PNG_SUCCESS)
	{
		Serial.print("[COVER] PNG openRAM failed err=");
		Serial.println(png->getLastError());
		delete png;
		return false;
	}

	const int imgW = png->getWidth();
	const int imgH = png->getHeight();
	context.srcW = static_cast<uint16_t>(imgW);
	context.srcH = static_cast<uint16_t>(imgH);

	int dstX = 0;
	int dstY = 0;
	int dstW = 0;
	int dstH = 0;
	computeAspectFitRect(imgW, imgH, display.width(), display.height(), dstX, dstY, dstW, dstH);
	context.dstX = static_cast<int16_t>(dstX);
	context.dstY = static_cast<int16_t>(dstY);
	context.dstW = static_cast<uint16_t>(dstW);
	context.dstH = static_cast<uint16_t>(dstH);

	context.scaleDiv = pickCoverScaleDiv(imgW, imgH, display.width(), display.height());

	const bool decodeOk = png->decode(&context, PNG_FAST_PALETTE) == PNG_SUCCESS;
	if (!decodeOk)
	{
		Serial.print("[COVER] PNG decode failed err=");
		Serial.println(png->getLastError());
	}
	if (context.pngLineBuffer)
	{
		free(context.pngLineBuffer);
		context.pngLineBuffer = nullptr;
		context.pngLineCapacity = 0;
	}
	png->close();
	delete png;
	return decodeOk;
}

void showReaderCoverOnDisplay()
{
	display.setRotation(1);
	display.setFullWindow();

	bool coverDecoded = false;
	if (readerHasCover && !readerCoverPath.isEmpty())
	{
		Serial.print("[COVER] attempting path: ");
		Serial.println(readerCoverPath);
		UNZIP* coverZip = new (std::nothrow) UNZIP();
		if (coverZip && coverZip->openZIP(readerBookPath.c_str(), epubOpenCallback, epubCloseCallback, epubReadCallback, epubSeekCallback) == 0)
		{
			if (extractNamedZipEntryToFile(*coverZip, readerCoverPath.c_str(), kReaderCoverTempPath))
			{
				Serial.print("[COVER] extracted to temp file: ");
				Serial.println(kReaderCoverTempPath);
				display.firstPage();
				do
				{
					display.fillScreen(GxEPD_WHITE);
					coverDecoded = drawCoverImageOnDisplay(kReaderCoverTempPath, readerCoverPath);
				}
				while (display.nextPage());
				LittleFS.remove(kReaderCoverTempPath);
			}
			else
			{
				Serial.println("[COVER] zip extract failed for path");
			}
			coverZip->closeZIP();
		}
		else
		{
			Serial.println("[COVER] openZIP failed");
		}

		if (coverZip)
		{
			delete coverZip;
		}
		LittleFS.remove(kReaderCoverTempPath);
	}

	if (!coverDecoded)
	{
		Serial.print("[COVER] showing placeholder (hasCover=");
		Serial.print(readerHasCover);
		Serial.print(", pathEmpty=");
		Serial.print(readerCoverPath.isEmpty());
		Serial.println(")");
		display.setRotation(1);
		display.setFullWindow();
		display.firstPage();
		do
		{
			display.fillScreen(GxEPD_WHITE);
			display.setTextColor(GxEPD_BLACK);
			setDisplayFont(ReaderFontStyle::Bold);
			display.setCursor(8, static_cast<int16_t>(readerFontSizePt + 12));
			display.print("Cover");

			setDisplayFont(ReaderFontStyle::Normal);
			display.setCursor(8, static_cast<int16_t>(readerFontSizePt + 30));
			display.print(truncateForDisplay(readerBookTitle, uiCharsPerLine(display.width() - 20)));
			display.setCursor(8, static_cast<int16_t>(readerFontSizePt + 30 + uiLineStep()));
			display.print("(No cover image found)");
		}
		while (display.nextPage());
	}
}

bool loadReaderChapter(uint16_t chapterIndex)
{
	if (chapterIndex >= readerChapterPaths.size())
	{
		return false;
	}

	closeReaderChapterStream();

	resetReaderPageBuilderState();
	resetReaderMarkupStreamState();
	readerChapterInputEnded = false;
	readerChapterFileOpen = false;

	epubZip = new (std::nothrow) UNZIP();
	if (!epubZip)
	{
		return false;
	}

	if (epubZip->openZIP(readerBookPath.c_str(), epubOpenCallback, epubCloseCallback, epubReadCallback, epubSeekCallback) != 0)
	{
		delete epubZip;
		epubZip = nullptr;
		return false;
	}

	auto cleanupZip = []()
	{
		if (epubZip)
		{
			if (readerChapterFileOpen)
			{
				epubZip->closeCurrentFile();
				readerChapterFileOpen = false;
			}
			epubZip->closeZIP();
			delete epubZip;
			epubZip = nullptr;
		}
	};

	if (!openNamedZipEntryForStreaming(*epubZip, readerChapterPaths[chapterIndex].c_str()))
	{
		cleanupZip();
		return false;
	}
	readerChapterFileOpen = true;
	readerChapterInputEnded = false;

	while (!readerChapterParseComplete)
	{
		if (appendNextReaderPage())
		{
			break;
		}
		if (readerChapterParseComplete)
		{
			break;
		}
	}

	if (readerPages.empty())
	{
		readerPages.push_back(encodeStyledLine("(No readable text found)", ReaderFontStyle::Oblique));
		readerChapterParseComplete = true;
	}
	readerChapterIndex = chapterIndex;
	readerPageIndex = 0;

	if (chapterIndex < kMaxBooks)
	{
		readerChapterPageCounts[chapterIndex] = readerPages.size();
	}
	return true;
}

bool appendNextReaderPage()
{
	if (readerChapterParseComplete)
	{
		return false;
	}
	if (drainReaderParsedTextBuffer())
	{
		return true;
	}

	if (readerChapterInputEnded)
	{
		finalizeReaderPageBuilder();
		return false;
	}

	if (!epubZip || !readerChapterFileOpen)
	{
		readerChapterInputEnded = true;
		finalizeReaderPageBuilder();
		return false;
	}

	uint8_t buffer[512];
	const int32_t bytesRead = epubZip->readCurrentFile(buffer, sizeof(buffer));
	if (bytesRead <= 0)
	{
		readerChapterInputEnded = true;
		if (!readerMarkupRawCarry.isEmpty())
		{
			for (size_t i = 0; i < readerMarkupRawCarry.length(); ++i)
			{
				feedReaderMarkupByte(readerMarkupRawCarry[i]);
			}
			readerMarkupRawCarry = "";
		}
		flushReaderMarkupTextSegment();
		if (readerMarkupState.inEntity)
		{
			readerMarkupTextSegment += '&';
			readerMarkupTextSegment += readerMarkupState.entityBuffer;
			readerMarkupState.inEntity = false;
			readerMarkupState.entityBuffer = "";
			flushReaderMarkupTextSegment();
		}
		finalizeReaderPageBuilder();
		closeReaderChapterStream();
		return false;
	}

	for (int32_t i = 0; i < bytesRead; ++i)
	{
		readerMarkupRawCarry += static_cast<char>(buffer[i]);
	}
	processReaderMarkupBuffer(false);
	if (drainReaderParsedTextBuffer())
	{
		return true;
	}

	return false;
}

bool ensureReaderPageBuilt(int32_t pageIndex)
{
	if (pageIndex < 0)
	{
		return false;
	}
	while (static_cast<int32_t>(readerPages.size()) <= pageIndex && !readerChapterParseComplete)
	{
		appendNextReaderPage();
	}
	return static_cast<int32_t>(readerPages.size()) > pageIndex;
}

void buildReaderPagesFromText(const String& text)
{
	resetReaderPageBuilderState();
	resetReaderMarkupStreamState();
	readerChapterInputEnded = true;
	readerParsedTextBuffer = text;

	if (readerParsedTextBuffer.isEmpty())
	{
		readerPages.push_back(encodeStyledLine("(No readable text found)", ReaderFontStyle::Oblique));
		readerChapterParseComplete = true;
		return;
	}

	drainReaderParsedTextBuffer();
	if (readerParsedTextBuffer.isEmpty())
	{
		finalizeReaderPageBuilder();
	}
}

bool openBookReader(const String& bookPath)
{
	const String normalizedPath = bookPath.startsWith("/") ? bookPath : "/" + bookPath;
	String bookTitle = normalizedPath;
	std::vector<String> chapterPaths;
	std::vector<String> chapterTitles;
	readerCoverPath = "";
	readerHasCover = false;
	readerShowingCover = false;
	if (!collectEpubChapterPaths(normalizedPath, bookTitle, chapterPaths, chapterTitles))
	{
		return false;
	}

	readerBookPath = normalizedPath;
	readerBookTitle = bookTitle;
	readerChapterPaths = chapterPaths;
	readerChapterTitles = chapterTitles;
	const uint32_t indexSignature = computeReaderIndexSignature(normalizedPath, chapterPaths);
	if (!tryLoadReaderIndexCache(indexSignature, static_cast<uint16_t>(chapterPaths.size())))
	{
		if (!buildReaderBookPageIndex())
		{
			return false;
		}
		saveReaderIndexCache(indexSignature, static_cast<uint16_t>(chapterPaths.size()), readerTotalPagesLoaded);
		Serial.println("Reader index: rebuilt and cached");
	}
	else
	{
		Serial.println("Reader index: loaded from cache");
	}
	readerChapterIndex = 0;
	readerPageIndex = 0;
	readerResumeChapterIndex = 0;
	readerResumePageIndex = 0;
	readerResumeShowingCover = readerHasCover;
	readerCoverReturnPageIndex = 0;

	uint16_t savedChapter = 0;
	uint16_t savedPage = 0;
	bool savedCover = false;
	if (loadReaderBookmark(normalizedPath, savedChapter, savedPage, savedCover))
	{
		if (savedChapter < readerChapterPaths.size())
		{
			readerResumeChapterIndex = savedChapter;
			readerResumePageIndex = savedPage;
		}
		readerResumeShowingCover = savedCover && readerHasCover;
	}

	readerJumpTargetChapterIndex = readerResumeChapterIndex;
	serialMode = ReaderSerialMode::ReaderStartMenu;
	return true;
}

void showReaderPageOnDisplay()
{
	if (readerPages.empty())
	{
		return;
	}

	display.setRotation(1);
	display.setFullWindow();
	display.firstPage();
	do
	{
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_BLACK);
		display.setTextWrap(false);

		setDisplayFont(ReaderFontStyle::Bold);
		display.setCursor(8, static_cast<int16_t>(readerFontSizePt + 6));
		display.print(truncateForDisplay(readerBookTitle, uiCharsPerLine(display.width() - 40)));

		setDisplayFont(ReaderFontStyle::Normal);
		display.setCursor(display.width() - 64, static_cast<int16_t>(readerFontSizePt + 6));
		uint32_t globalPageNumber = 0;
		for (uint16_t i = 0; i < readerChapterIndex; ++i)
		{
			globalPageNumber += readerChapterPageCounts[i];
		}
		globalPageNumber += readerPageIndex + 1;
		display.print(globalPageNumber);
		display.print("/");
		display.print(readerTotalPagesLoaded);

		int16_t y = static_cast<int16_t>(uiReaderBodyStartY());
		const String& pageText = readerPages[readerPageIndex];
		int start = 0;
		while (start <= pageText.length())
		{
			int end = pageText.indexOf('\n', start);
			String encodedLine = end >= 0 ? pageText.substring(start, end) : pageText.substring(start);
			ReaderTextAlign lineAlign = ReaderTextAlign::Left;
			String styledLine;
			decodeLineAlignment(encodedLine, lineAlign, styledLine);
			const uint16_t lineWidth = measureEncodedTextWidth(styledLine);
			const int16_t leftX = 8;
			const int16_t rightLimit = static_cast<int16_t>(display.width() - 8);
			int16_t lineX = leftX;
			if (lineAlign == ReaderTextAlign::Center)
			{
				lineX = static_cast<int16_t>((display.width() - static_cast<int16_t>(lineWidth)) / 2);
				if (lineX < leftX)
				{
					lineX = leftX;
				}
			}
			else if (lineAlign == ReaderTextAlign::Right)
			{
				lineX = static_cast<int16_t>(rightLimit - static_cast<int16_t>(lineWidth));
				if (lineX < leftX)
				{
					lineX = leftX;
				}
			}

			drawEncodedTextLine(lineX, y, styledLine);
			y += static_cast<int16_t>(uiLineStep());
			if (y > display.height() - 8)
			{
				break;
			}
			if (end < 0)
			{
				break;
			}
			start = end + 1;
		}
	}
	while (display.nextPage());
}

void printReaderHelpToSerial()
{
	Serial.println();
	Serial.println("===== Reader =====");
	Serial.println("down -> Next page");
	Serial.println("up -> Previous page");
	Serial.println("left -> Cover/Main menu");
	uint32_t globalPageNumber = 0;
	for (uint16_t i = 0; i < readerChapterIndex; ++i)
	{
		globalPageNumber += readerChapterPageCounts[i];
	}
	if (readerShowingCover)
	{
		Serial.println("Page Cover");
	}
	else
	{
		globalPageNumber += readerPageIndex + 1;
		Serial.print("Page ");
		Serial.print(globalPageNumber);
		Serial.print("/");
		Serial.println(readerTotalPagesLoaded);
	}

	if (!readerChapterParseComplete)
	{
		Serial.println("(more pages in this chapter load on demand)");
	}
	Serial.print("Chapter ");
	Serial.print(readerChapterIndex + 1);
	Serial.print("/");
	Serial.println(readerChapterPaths.size());
	Serial.println("Controls: up/down page, left back");
	Serial.print("> ");
}

bool openReaderAtPosition(uint16_t chapterIndex, uint16_t pageIndex, bool showCover)
{
	if (chapterIndex >= readerChapterPaths.size())
	{
		chapterIndex = 0;
		pageIndex = 0;
		showCover = readerHasCover;
	}

	if (!loadReaderChapter(chapterIndex))
	{
		return false;
	}

	Serial.print("[OPEN] Cover: hasCover=");
	Serial.print(readerHasCover);
	Serial.print(" path='");
	Serial.print(readerCoverPath);
	Serial.print("' showCover=");
	Serial.println(showCover);

	readerShowingCover = showCover && readerHasCover;
	readerCoverReturnPageIndex = pageIndex;
	serialMode = ReaderSerialMode::ReaderMenu;
	if (readerShowingCover)
	{
		Serial.println("[OPEN] Showing cover");
		showReaderCoverOnDisplay();
		saveCurrentReaderProgress();
		printReaderHelpToSerial();
	}
	else
	{
		goToReaderPage(static_cast<int32_t>(pageIndex));
	}
	return true;
}

void showReaderStartMenuOnDisplay()
{
	clampMenuCursor(readerStartMenuCursor, readerStartMenuOptionCount());

	display.setRotation(1);
	display.setFullWindow();
	display.firstPage();
	do
	{
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_BLACK);

		setDisplayFont(ReaderFontStyle::Bold);
		display.setCursor(8, uiScreenHeaderY());
		display.print("Book menu");

		setDisplayFont(ReaderFontStyle::Normal);
		int16_t y = uiScreenBodyStartY();
		const uint16_t maxChars = uiCharsPerLine(display.width() - 32);
		uint16_t optionIndex = 0;

		String resumeLine;
		if (readerResumeShowingCover)
		{
			resumeLine = "Resume: Cover";
		}
		else
		{
			const uint32_t resumeGlobal = globalPageForPosition(readerResumeChapterIndex, readerResumePageIndex, false);
			resumeLine = "Resume: page " + String(resumeGlobal) + "/" + String(readerTotalPagesLoaded);
		}
		if (optionIndex == readerStartMenuCursor)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print(truncateForDisplay(resumeLine, maxChars));
		y += static_cast<int16_t>(uiLineStep());
		++optionIndex;

		if (optionIndex == readerStartMenuCursor)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Jump to page");
		y += static_cast<int16_t>(uiLineStep());
		++optionIndex;

		if (optionIndex == readerStartMenuCursor)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Back");
		y += static_cast<int16_t>(uiLineStep());
		++optionIndex;

		for (uint16_t i = 0; i < readerChapterPaths.size(); ++i)
		{
			if (y > display.height() - 8)
			{
				break;
			}
			if (optionIndex == readerStartMenuCursor)
			{
				drawMenuCursorTriangle(y);
			}
			String line = chapterLabelForDisplay(i);
			display.setCursor(20, y);
			display.print(truncateForDisplay(line, maxChars));
			y += static_cast<int16_t>(uiLineStep());
			++optionIndex;
		}
	}
	while (display.nextPage());
}

void printReaderStartMenuToSerial()
{
	clampMenuCursor(readerStartMenuCursor, readerStartMenuOptionCount());

	Serial.println();
	Serial.println("===== Book Menu =====");
	if (readerResumeShowingCover)
	{
		printSerialMenuLine(readerStartMenuCursor == 0, "Resume at cover");
	}
	else
	{
		const uint32_t resumeGlobal = globalPageForPosition(readerResumeChapterIndex, readerResumePageIndex, false);
		printSerialMenuLine(readerStartMenuCursor == 0, "Resume at page " + String(resumeGlobal) + "/" + String(readerTotalPagesLoaded));
	}
	printSerialMenuLine(readerStartMenuCursor == 1, "Jump to page");
	printSerialMenuLine(readerStartMenuCursor == 2, "Back to main menu");
	for (uint16_t i = 0; i < readerChapterPaths.size(); ++i)
	{
		printSerialMenuLine(readerStartMenuCursor == static_cast<uint16_t>(i + 3), chapterLabelForDisplay(i));
	}
	Serial.println("Controls: up/down move, right select, left back");
	Serial.print("> ");
}

void showReaderStartMenu()
{
	serialMode = ReaderSerialMode::ReaderStartMenu;
	readerStartMenuCursor = 0;
	showReaderStartMenuOnDisplay();
	printReaderStartMenuToSerial();
}

void promptReaderJumpToPage()
{
	readerJumpTargetChapterIndex = readerResumeChapterIndex;
	serialMode = ReaderSerialMode::ReaderJumpPageInput;

	display.setRotation(1);
	display.setFullWindow();
	display.firstPage();
	do
	{
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_BLACK);
		setDisplayFont(ReaderFontStyle::Bold);
		display.setCursor(8, uiScreenHeaderY());
		display.print("Jump to page");

		setDisplayFont(ReaderFontStyle::Normal);
		display.setCursor(8, uiScreenBodyStartY());
		display.print("Send page number");
		display.setCursor(8, static_cast<int16_t>(uiScreenBodyStartY() + static_cast<int16_t>(uiLineStep())));
		display.print("via serial now");
	}
	while (display.nextPage());

	Serial.println();
	Serial.println("Enter page number to jump to (1-based), or left to go back:");
	Serial.print("> ");
}

void handleReaderStartSelection(uint16_t choice)
{
	if (choice == 1)
	{
		if (!openReaderAtPosition(readerResumeChapterIndex, readerResumePageIndex, readerResumeShowingCover))
		{
			Serial.println("Failed to open chapter");
			showStatusOnDisplay("EPUB error", "Failed to open chapter");
			delay(1000);
			showMainMenu();
		}
		return;
	}

	if (choice == 2)
	{
		promptReaderJumpToPage();
		return;
	}

	if (choice == 3)
	{
		clearReaderState();
		showMainMenu();
		return;
	}

	if (choice >= 4)
	{
		const uint16_t chapterIndex = static_cast<uint16_t>(choice - 4);
		if (chapterIndex < readerChapterPaths.size())
		{
			if (!openReaderAtPosition(chapterIndex, 0, false))
			{
				Serial.println("Failed to open chapter");
				showStatusOnDisplay("EPUB error", "Failed to open chapter");
				delay(1000);
				showMainMenu();
			}
			return;
		}
	}

	Serial.println("Invalid option");
	showInvalidOptionOnDisplay();
	delay(800);
	showReaderStartMenu();
}

void handleReaderJumpPageSelection(uint16_t pageNumber)
{
	if (pageNumber == 0)
	{
		Serial.println("Invalid page number");
		showInvalidOptionOnDisplay();
		delay(800);
		showReaderStartMenu();
		return;
	}

	if (readerTotalPagesLoaded == 0)
	{
		readerTotalPagesLoaded = 0;
		for (uint16_t i = 0; i < readerChapterPaths.size() && i < kMaxBooks; ++i)
		{
			readerTotalPagesLoaded += readerChapterPageCounts[i];
		}
	}

	if (pageNumber > readerTotalPagesLoaded)
	{
		pageNumber = static_cast<uint16_t>(readerTotalPagesLoaded);
	}

	uint16_t globalPageTarget = pageNumber;
	uint16_t targetChapter = 0;
	uint16_t pageInChapter = globalPageTarget;

	for (uint16_t i = 0; i < readerChapterPaths.size(); ++i)
	{
		if (readerChapterPageCounts[i] == 0)
		{
			continue;
		}
		if (pageInChapter <= readerChapterPageCounts[i])
		{
			targetChapter = i;
			break;
		}
		pageInChapter -= readerChapterPageCounts[i];
		targetChapter = i + 1;
	}

	if (targetChapter >= readerChapterPaths.size())
	{
		targetChapter = readerChapterPaths.size() - 1;
		pageInChapter = readerChapterPageCounts[targetChapter];
	}

	if (pageInChapter == 0)
	{
		pageInChapter = 1;
	}

	if (!openReaderAtPosition(targetChapter, static_cast<uint16_t>(pageInChapter - 1), false))
	{
		Serial.println("Failed to open chapter");
		showStatusOnDisplay("EPUB error", "Failed to open chapter");
		delay(1000);
		showMainMenu();
		return;
	}
}

void goToReaderPage(int32_t pageIndex)
{
	if (readerShowingCover)
	{
		if (pageIndex <= 0)
		{
			showReaderCoverOnDisplay();
			saveCurrentReaderProgress();
			printReaderHelpToSerial();
			return;
		}

		readerShowingCover = false;
		pageIndex -= 1;
	}

	if (readerPages.empty())
	{
		return;
	}
	if (pageIndex < 0)
	{
		if (readerChapterIndex > 0 && loadReaderChapter(readerChapterIndex - 1))
		{
			while (!readerChapterParseComplete)
			{
				appendNextReaderPage();
			}
			pageIndex = static_cast<int32_t>(readerPages.size()) - 1;
		}
		else if (readerHasCover)
		{
			readerShowingCover = true;
			showReaderCoverOnDisplay();
			saveCurrentReaderProgress();
			printReaderHelpToSerial();
			return;
		}
		else
		{
			pageIndex = 0;
		}
	}
	if (pageIndex >= static_cast<int32_t>(readerPages.size()))
	{
		if (ensureReaderPageBuilt(pageIndex))
		{
			// requested page has now been built within current chapter
		}
		else if (readerChapterIndex + 1 < readerChapterPaths.size() && loadReaderChapter(readerChapterIndex + 1))
		{
			pageIndex = 0;
		}
		else
		{
			pageIndex = static_cast<int32_t>(readerPages.size()) - 1;
		}
	}
	readerPageIndex = static_cast<uint16_t>(pageIndex);
	showReaderPageOnDisplay();
	saveCurrentReaderProgress();
	printReaderHelpToSerial();
}

String buildBookListHtml()
{
	String html;
	html.reserve(4096);
	html += "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
	html += "<title>Ebook Reader Upload</title>";
	html += "<style>body{font-family:system-ui,-apple-system,sans-serif;max-width:800px;margin:2rem auto;padding:0 1rem;line-height:1.5;background:#f7f3ea;color:#1f1f1f}h1{margin-bottom:.25rem}form{padding:1rem;border:1px solid #d4c9b8;border-radius:14px;background:#fffaf2}ul{padding-left:1.25rem}li{margin:.25rem 0}code{background:#eee6d8;padding:.1rem .35rem;border-radius:4px}</style></head><body>";
	html += "<h1>Ebook Reader</h1>";
	html += "<p>Upload books to the device. The Wi-Fi server will shut down after the upload finishes.</p>";
	html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
	html += "<input type='file' name='book' multiple accept='.txt,.epub,.mobi,.pdf,.html,.htm,.fb2,.rtf,.md'>";
	html += "<button type='submit'>Upload</button></form>";
	html += "<h2>Books on device</h2><ul>";

	File root = LittleFS.open("/");
	File file = root.openNextFile();
	if (!file)
	{
		html += "<li>No books uploaded yet.</li>";
	}
	while (file)
	{
		if (file.isDirectory())
		{
			file = root.openNextFile();
			continue;
		}
		html += "<li><code>";
		html += htmlEscape(String(file.name()));
		html += "</code> (";
		html += String(file.size());
		html += " bytes)</li>";
		file = root.openNextFile();
	}

	html += "</ul></body></html>";
	return html;
}

uint16_t refreshBookList()
{
	bookCount = 0;
	File root = LittleFS.open("/");
	if (!root)
	{
		return 0;
	}

	File file = root.openNextFile();
	while (file && bookCount < kMaxBooks)
	{
		if (file.isDirectory())
		{
			file = root.openNextFile();
			continue;
		}
		bookNames[bookCount] = String(file.name());
		++bookCount;
		file = root.openNextFile();
	}
	return bookCount;
}

void printMainMenuToSerial()
{
	clampMenuCursor(mainMenuCursor, mainMenuOptionCount());

	Serial.println();
	Serial.println("===== Ebook Reader =====");
	printSerialMenuLine(mainMenuCursor == 0, "Upload books");
	printSerialMenuLine(mainMenuCursor == 1, "Delete books");
	printSerialMenuLine(mainMenuCursor == 2, "Settings");
	for (uint16_t i = 0; i < bookCount; ++i)
	{
		printSerialMenuLine(mainMenuCursor == static_cast<uint16_t>(i + 3), bookNames[i]);
	}
	if (bookCount == 0)
	{
		Serial.println("(No books uploaded)");
	}
	Serial.println("Controls: up/down move, right select");
	Serial.print("> ");
}

void printSettingsMenuToSerial()
{
	clampMenuCursor(settingsMenuCursor, settingsMenuOptionCount());

	Serial.println();
	Serial.println("===== Settings =====");
	printSerialMenuLine(settingsMenuCursor == 0, "Back");
	printSerialMenuLine(settingsMenuCursor == 1, "Font family: " + String(fontFamilyLabel()));
	printSerialMenuLine(settingsMenuCursor == 2, "Font size: " + String(readerFontSizePt));
	printSerialMenuLine(settingsMenuCursor == 3, "Line spacing: " + String(lineSpacingLabel()));
	printSerialMenuLine(settingsMenuCursor == 4, "Set family to Sans serif");
	printSerialMenuLine(settingsMenuCursor == 5, "Set family to Serif");
	printSerialMenuLine(settingsMenuCursor == 6, "Set family to Mono");
	printSerialMenuLine(settingsMenuCursor == 7, "Set size to 9");
	printSerialMenuLine(settingsMenuCursor == 8, "Set size to 12");
	printSerialMenuLine(settingsMenuCursor == 9, "Set size to 18");
	printSerialMenuLine(settingsMenuCursor == 10, "Set size to 24");
	Serial.println("Controls: up/down move, right select, left back");
	Serial.print("> ");
}

void printDeleteMenuToSerial()
{
	clampMenuCursor(deleteMenuCursor, deleteMenuOptionCount());

	Serial.println();
	Serial.println("===== Delete Books =====");
	if (bookCount == 0)
	{
		Serial.println("No books to delete.");
		Serial.println("Returning to main menu.");
		return;
	}

	printSerialMenuLine(deleteMenuCursor == 0, "Cancel");
	for (uint16_t i = 0; i < bookCount; ++i)
	{
		printSerialMenuLine(deleteMenuCursor == static_cast<uint16_t>(i + 1), bookNames[i]);
	}
	Serial.println("Controls: up/down move, right select, left back");
	Serial.print("> ");
}

void showMainMenuOnDisplay()
{
	clampMenuCursor(mainMenuCursor, mainMenuOptionCount());

	display.setRotation(1);
	display.setFullWindow();
	display.firstPage();
	do
	{
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_BLACK);

		setDisplayFont(ReaderFontStyle::Bold);
		display.setCursor(8, uiScreenHeaderY());
		display.print("Ebook Reader Menu");

		setDisplayFont(ReaderFontStyle::Normal);
		int16_t y = uiScreenBodyStartY();
		const uint16_t maxCharacters = uiCharsPerLine(display.width() - 32);
		if (mainMenuCursor == 0)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Upload books");
		y += static_cast<int16_t>(uiLineStep());
		if (mainMenuCursor == 1)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Delete books");
		y += static_cast<int16_t>(uiLineStep());
		if (mainMenuCursor == 2)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Settings");
		y += static_cast<int16_t>(uiLineStep());

		if (bookCount == 0)
		{
			display.setCursor(20, y);
			display.print("No books uploaded yet");
		}

		for (uint16_t i = 0; i < bookCount; ++i)
		{
			if (mainMenuCursor == static_cast<uint16_t>(i + 3))
			{
				drawMenuCursorTriangle(y);
			}
			display.setCursor(20, y);
			display.print(truncateForDisplay(bookNames[i], maxCharacters));
			y += static_cast<int16_t>(uiLineStep());
			if (y > display.height() - 8)
			{
				break;
			}
		}
	}
	while (display.nextPage());
}

void showDeleteMenuOnDisplay()
{
	clampMenuCursor(deleteMenuCursor, deleteMenuOptionCount());

	display.setRotation(1);
	display.setFullWindow();
	display.firstPage();
	do
	{
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_BLACK);
		setDisplayFont(ReaderFontStyle::Bold);
		display.setCursor(8, uiScreenHeaderY());
		display.print("Delete books");

		setDisplayFont(ReaderFontStyle::Normal);
		int16_t y = uiScreenBodyStartY();
		const uint16_t maxCharacters = uiCharsPerLine(display.width() - 32);
		if (deleteMenuCursor == 0)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Cancel");
		y += static_cast<int16_t>(uiLineStep());

		if (bookCount == 0)
		{
			display.setCursor(20, y);
			display.print("No books to delete");
		}

		for (uint16_t i = 0; i < bookCount; ++i)
		{
			if (deleteMenuCursor == static_cast<uint16_t>(i + 1))
			{
				drawMenuCursorTriangle(y);
			}
			display.setCursor(20, y);
			display.print(truncateForDisplay(bookNames[i], maxCharacters));
			y += static_cast<int16_t>(uiLineStep());
			if (y > display.height() - 8)
			{
				break;
			}
		}
	}
	while (display.nextPage());
}

void showStatusOnDisplay(const String& title, const String& line)
{
	display.setRotation(1);
	display.setFullWindow();
	display.firstPage();
	do
	{
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_BLACK);
		setDisplayFont(ReaderFontStyle::Bold);
		display.setCursor(8, uiScreenHeaderY());
		display.print(title);

		setDisplayFont(ReaderFontStyle::Normal);
		display.setCursor(8, uiScreenBodyStartY());
		display.print(truncateForDisplay(line, uiCharsPerLine(display.width() - 20)));
	}
	while (display.nextPage());
}

void showBookSelectedOnDisplay(const String& bookName)
{
	display.setRotation(1);
	display.setFullWindow();
	display.firstPage();
	do
	{
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_BLACK);
		setDisplayFont(ReaderFontStyle::Bold);
		display.setCursor(8, uiScreenHeaderY());
		display.print("Selected book");

		setDisplayFont(ReaderFontStyle::Normal);
		display.setCursor(8, uiScreenBodyStartY());
		display.print(truncateForDisplay(bookName, uiCharsPerLine(display.width() - 20)));
		display.setCursor(8, static_cast<int16_t>(uiScreenBodyStartY() + static_cast<int16_t>(uiLineStep())));
		display.print("(Reader view next step)");
	}
	while (display.nextPage());
}

void showInvalidOptionOnDisplay()
{
	display.setRotation(1);
	display.setFullWindow();
	display.firstPage();
	do
	{
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_BLACK);
		setDisplayFont(ReaderFontStyle::Bold);
		display.setCursor(8, uiScreenHeaderY());
		display.print("Invalid option");

		setDisplayFont(ReaderFontStyle::Normal);
		display.setCursor(8, uiScreenBodyStartY());
		display.print("Use up/down/left/right.");
	}
	while (display.nextPage());
}

void showUploadPortalOnDisplay(const IPAddress& ipAddress)
{
	display.setRotation(1);
	display.setFullWindow();
	display.firstPage();
	do
	{
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_BLACK);

		setDisplayFont(ReaderFontStyle::Bold);
		display.setCursor(8, uiScreenHeaderY());
		display.print("Wi-Fi upload ready");

		setDisplayFont(ReaderFontStyle::Normal);
		display.setCursor(8, uiScreenBodyStartY());
		display.print("SSID: ");
		display.println(kApSsid);
		display.print("IP: ");
		display.println(ipAddress);
		display.println();
		display.println("Upload books from the web page.");
		display.println("Server turns off after upload.");
		display.println();
		display.println("Existing books:");

		File root = LittleFS.open("/");
		File file = root.openNextFile();
		uint16_t count = 0;
		const uint16_t maxCharacters = uiCharsPerLine(display.width() - 20);
		while (file && count < 12)
		{
			if (file.isDirectory())
			{
				file = root.openNextFile();
				continue;
			}
			display.print("- ");
			display.println(truncateForDisplay(String(file.name()), maxCharacters - 2));
			++count;
			file = root.openNextFile();
		}
		if (count == 0)
		{
			display.println("- none");
		}
	}
	while (display.nextPage());
}

void showSettingsMenuOnDisplay()
{
	clampMenuCursor(settingsMenuCursor, settingsMenuOptionCount());

	display.setRotation(1);
	display.setFullWindow();
	display.firstPage();
	do
	{
		display.fillScreen(GxEPD_WHITE);
		display.setTextColor(GxEPD_BLACK);
		setDisplayFont(ReaderFontStyle::Bold);
		display.setCursor(8, uiScreenHeaderY());
		display.print("Settings");

		setDisplayFont(ReaderFontStyle::Normal);
		int16_t y = uiScreenBodyStartY();
		const uint16_t maxChars = uiCharsPerLine(display.width() - 32);
		if (settingsMenuCursor == 0)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Back");
		y += static_cast<int16_t>(uiLineStep());
		if (settingsMenuCursor == 1)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Family: ");
		display.print(fontFamilyLabel());
		y += static_cast<int16_t>(uiLineStep());
		if (settingsMenuCursor == 2)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Size: ");
		display.print(readerFontSizePt);
		y += static_cast<int16_t>(uiLineStep());
		if (settingsMenuCursor == 3)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Line spacing: ");
		display.print(lineSpacingLabel());
		y += static_cast<int16_t>(uiLineStep());
		if (settingsMenuCursor == 4)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Sans serif");
		y += static_cast<int16_t>(uiLineStep());
		if (settingsMenuCursor == 5)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Serif");
		y += static_cast<int16_t>(uiLineStep());
		if (settingsMenuCursor == 6)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print("Mono");
		y += static_cast<int16_t>(uiLineStep());
		if (settingsMenuCursor == 7)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print(truncateForDisplay("Size to 9", maxChars));
		y += static_cast<int16_t>(uiLineStep());
		if (settingsMenuCursor == 8)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print(truncateForDisplay("Size to 12", maxChars));
		y += static_cast<int16_t>(uiLineStep());
		if (settingsMenuCursor == 9)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print(truncateForDisplay("Size to 18", maxChars));
		y += static_cast<int16_t>(uiLineStep());
		if (settingsMenuCursor == 10)
		{
			drawMenuCursorTriangle(y);
		}
		display.setCursor(20, y);
		display.print(truncateForDisplay("Size to 24", maxChars));
	}
	while (display.nextPage());
}

void handleRoot()
{
	server.send(200, "text/html", buildBookListHtml());
}

void handleUploadFinished()
{
	const String message = uploadSucceeded
		? "Upload complete. The Wi-Fi server is shutting down now."
		: "Upload finished without a saved file.";
	server.send(200, "text/html", buildBookListHtml() + "<p><strong>" + htmlEscape(message) + "</strong></p>");
	if (uploadSucceeded)
	{
		shutdownRequested = true;
	}
}

void handleFileUpload()
{
	HTTPUpload& upload = server.upload();

	if (upload.status == UPLOAD_FILE_START)
	{
		uploadSucceeded = false;
		uploadPath = sanitizeFilename(upload.filename);
		if (LittleFS.exists(uploadPath))
		{
			LittleFS.remove(uploadPath);
		}
		uploadFile = LittleFS.open(uploadPath, FILE_WRITE);
	}
	else if (upload.status == UPLOAD_FILE_WRITE)
	{
		if (uploadFile)
		{
			uploadFile.write(upload.buf, upload.currentSize);
		}
	}
	else if (upload.status == UPLOAD_FILE_END)
	{
		if (uploadFile)
		{
			uploadFile.close();
			uploadSucceeded = true;
		}
	}
	else if (upload.status == UPLOAD_FILE_ABORTED)
	{
		if (uploadFile)
		{
			uploadFile.close();
		}
		if (!uploadPath.isEmpty())
		{
			LittleFS.remove(uploadPath);
		}
		uploadSucceeded = false;
	}
}

void startWifiServer()
{
	WiFi.mode(WIFI_AP);
	WiFi.softAP(kApSsid, kApPassword);

	server.on("/", HTTP_GET, handleRoot);
	server.on("/upload", HTTP_POST, handleUploadFinished, handleFileUpload);
	server.onNotFound([]()
	{
		server.send(404, "text/plain", "Not found");
	});
	server.begin();
	wifiServerRunning = true;

	Serial.println();
	Serial.println("Upload server started");
	Serial.print("Connect to SSID: ");
	Serial.println(kApSsid);
	Serial.print("Open: http://");
	Serial.println(WiFi.softAPIP());
	Serial.println("After upload, server will stop and return to menu.");

	showUploadPortalOnDisplay(WiFi.softAPIP());
}

void stopWifiServer()
{
	if (!wifiServerRunning)
	{
		return;
	}

	server.stop();
	WiFi.softAPdisconnect(true);
	WiFi.mode(WIFI_OFF);
	wifiServerRunning = false;
	Serial.println("Upload server stopped");
}

void showMainMenu()
{
	serialMode = ReaderSerialMode::MainMenu;
	refreshBookList();
	mainMenuCursor = 0;
	showMainMenuOnDisplay();
	printMainMenuToSerial();
}

void showDeleteMenu()
{
	serialMode = ReaderSerialMode::DeleteMenu;
	refreshBookList();
	deleteMenuCursor = 0;
	if (bookCount == 0)
	{
		showStatusOnDisplay("Delete books", "No books to delete");
		delay(800);
		showMainMenu();
		return;
	}
	showDeleteMenuOnDisplay();
	printDeleteMenuToSerial();
}

void showSettingsMenu()
{
	serialMode = ReaderSerialMode::SettingsMenu;
	settingsMenuCursor = 0;
	showSettingsMenuOnDisplay();
	printSettingsMenuToSerial();
}

void handleSettingsSelection(uint16_t choice)
{
	if (choice == 1)
	{
		showMainMenu();
		return;
	}

	if (choice == 2)
	{
		if (readerFontFamily == ReaderFontFamily::SansSerif)
		{
			readerFontFamily = ReaderFontFamily::Serif;
		}
		else if (readerFontFamily == ReaderFontFamily::Serif)
		{
			readerFontFamily = ReaderFontFamily::Mono;
		}
		else
		{
			readerFontFamily = ReaderFontFamily::SansSerif;
		}
		saveReaderSettings();
		showSettingsMenu();
		return;
	}

	if (choice == 3)
	{
		if (readerFontSizePt == 9) readerFontSizePt = 12;
		else if (readerFontSizePt == 12) readerFontSizePt = 18;
		else if (readerFontSizePt == 18) readerFontSizePt = 24;
		else readerFontSizePt = 9;
		saveReaderSettings();
		showSettingsMenu();
		return;
	}

	if (choice == 4)
	{
		cycleLineSpacing();
		saveReaderSettings();
		showSettingsMenu();
		return;
	}

	if (choice == 5)
	{
		readerFontFamily = ReaderFontFamily::SansSerif;
		saveReaderSettings();
		showSettingsMenu();
		return;
	}

	if (choice == 6)
	{
		readerFontFamily = ReaderFontFamily::Serif;
		saveReaderSettings();
		showSettingsMenu();
		return;
	}

	if (choice == 7)
	{
		readerFontFamily = ReaderFontFamily::Mono;
		saveReaderSettings();
		showSettingsMenu();
		return;
	}

	if (choice == 8 || choice == 9 || choice == 10 || choice == 11)
	{
		readerFontSizePt = (choice == 8) ? 9 : (choice == 9) ? 12 : (choice == 10) ? 18 : 24;
		saveReaderSettings();
		showSettingsMenu();
		return;
	}

	Serial.println("Invalid option");
	showInvalidOptionOnDisplay();
	delay(800);
	showSettingsMenu();
}

void handleDeleteSelection(uint16_t choice)
{
	if (choice == 1)
	{
		showMainMenu();
		return;
	}

	if (choice >= 2 && choice < bookCount + 2)
	{
		const String selectedBook = bookNames[choice - 2];
		const String deletePath = selectedBook.startsWith("/") ? selectedBook : "/" + selectedBook;
		if (LittleFS.remove(deletePath.c_str()))
		{
			Serial.print("Deleted: ");
			Serial.println(deletePath);
			showStatusOnDisplay("Book deleted", selectedBook);
			delay(800);
		}
		else
		{
			Serial.print("Delete failed: ");
			Serial.println(deletePath);
			showStatusOnDisplay("Delete failed", selectedBook);
			delay(1000);
		}
		showMainMenu();
		return;
	}

	Serial.println("Invalid option");
	showInvalidOptionOnDisplay();
	delay(800);
	showDeleteMenu();
}

void handleMenuSelection(uint16_t choice)
{
	if (choice == 1)
	{
		startWifiServer();
		return;
	}

	if (choice == 2)
	{
		showDeleteMenu();
		return;
	}

	if (choice == 3)
	{
		showSettingsMenu();
		return;
	}

	if (choice >= 4 && choice < bookCount + 4)
	{
		const String selectedBook = bookNames[choice - 4];
		Serial.print("Selected: ");
		Serial.println(selectedBook);
		if (openBookReader(selectedBook))
		{
			showReaderStartMenu();
		}
		else
		{
			Serial.println("Failed to open EPUB");
			showStatusOnDisplay("EPUB error", selectedBook);
			delay(1000);
			showMainMenu();
		}
		return;
	}

	Serial.println("Invalid option");
	showInvalidOptionOnDisplay();
	delay(800);
	showMainMenu();
}

void handleReaderSelection(uint16_t choice)
{
	if (choice == 1)
	{
		goToReaderPage(static_cast<int32_t>(readerPageIndex) + 1);
		return;
	}

	if (choice == 2)
	{
		goToReaderPage(static_cast<int32_t>(readerPageIndex) - 1);
		return;
	}

	if (choice == 3)
	{
		clearReaderState();
		showMainMenu();
		return;
	}

	Serial.println("Invalid option");
	showInvalidOptionOnDisplay();
	delay(800);
	printReaderHelpToSerial();
}

bool handleSerialNavInput(SerialNavInput nav)
{
	if (nav == SerialNavInput::None)
	{
		return false;
	}

	if (serialMode == ReaderSerialMode::MainMenu)
	{
		if (nav == SerialNavInput::Up)
		{
			moveMenuCursor(mainMenuCursor, mainMenuOptionCount(), -1);
			showMainMenuOnDisplay();
			printMainMenuToSerial();
			return true;
		}
		if (nav == SerialNavInput::Down)
		{
			moveMenuCursor(mainMenuCursor, mainMenuOptionCount(), 1);
			showMainMenuOnDisplay();
			printMainMenuToSerial();
			return true;
		}
		if (nav == SerialNavInput::Right)
		{
			if (mainMenuCursor >= 3 && mainMenuCursor < static_cast<uint16_t>(3 + bookCount))
			{
				const String selectedBook = bookNames[mainMenuCursor - 3];
				Serial.print("Selected: ");
				Serial.println(selectedBook);
				if (openBookReader(selectedBook))
				{
					if (!openReaderAtPosition(readerResumeChapterIndex, readerResumePageIndex, readerHasCover))
					{
						Serial.println("Failed to open EPUB");
						showStatusOnDisplay("EPUB error", selectedBook);
						delay(1000);
						showMainMenu();
					}
				}
				else
				{
					Serial.println("Failed to open EPUB");
					showStatusOnDisplay("EPUB error", selectedBook);
					delay(1000);
					showMainMenu();
				}
				return true;
			}

			handleMenuSelection(static_cast<uint16_t>(mainMenuCursor + 1));
			return true;
		}
		if (nav == SerialNavInput::Left)
		{
			if (mainMenuCursor >= 3 && mainMenuCursor < static_cast<uint16_t>(3 + bookCount))
			{
				const String selectedBook = bookNames[mainMenuCursor - 3];
				Serial.print("Selected: ");
				Serial.println(selectedBook);
				if (openBookReader(selectedBook))
				{
					showReaderStartMenu();
				}
				else
				{
					Serial.println("Failed to open EPUB");
					showStatusOnDisplay("EPUB error", selectedBook);
					delay(1000);
					showMainMenu();
				}
				return true;
			}

			printMainMenuToSerial();
			return true;
		}
	}

	if (serialMode == ReaderSerialMode::DeleteMenu)
	{
		if (nav == SerialNavInput::Left)
		{
			showMainMenu();
			return true;
		}
		if (nav == SerialNavInput::Up)
		{
			moveMenuCursor(deleteMenuCursor, deleteMenuOptionCount(), -1);
			showDeleteMenuOnDisplay();
			printDeleteMenuToSerial();
			return true;
		}
		if (nav == SerialNavInput::Down)
		{
			moveMenuCursor(deleteMenuCursor, deleteMenuOptionCount(), 1);
			showDeleteMenuOnDisplay();
			printDeleteMenuToSerial();
			return true;
		}
		if (nav == SerialNavInput::Right)
		{
			handleDeleteSelection(static_cast<uint16_t>(deleteMenuCursor + 1));
			return true;
		}
	}

	if (serialMode == ReaderSerialMode::SettingsMenu)
	{
		if (nav == SerialNavInput::Left)
		{
			showMainMenu();
			return true;
		}
		if (nav == SerialNavInput::Up)
		{
			moveMenuCursor(settingsMenuCursor, settingsMenuOptionCount(), -1);
			showSettingsMenuOnDisplay();
			printSettingsMenuToSerial();
			return true;
		}
		if (nav == SerialNavInput::Down)
		{
			moveMenuCursor(settingsMenuCursor, settingsMenuOptionCount(), 1);
			showSettingsMenuOnDisplay();
			printSettingsMenuToSerial();
			return true;
		}
		if (nav == SerialNavInput::Right)
		{
			handleSettingsSelection(static_cast<uint16_t>(settingsMenuCursor + 1));
			return true;
		}
	}

	if (serialMode == ReaderSerialMode::ReaderStartMenu)
	{
		if (nav == SerialNavInput::Left)
		{
			clearReaderState();
			showMainMenu();
			return true;
		}
		if (nav == SerialNavInput::Up)
		{
			moveMenuCursor(readerStartMenuCursor, readerStartMenuOptionCount(), -1);
			showReaderStartMenuOnDisplay();
			printReaderStartMenuToSerial();
			return true;
		}
		if (nav == SerialNavInput::Down)
		{
			moveMenuCursor(readerStartMenuCursor, readerStartMenuOptionCount(), 1);
			showReaderStartMenuOnDisplay();
			printReaderStartMenuToSerial();
			return true;
		}
		if (nav == SerialNavInput::Right)
		{
			handleReaderStartSelection(static_cast<uint16_t>(readerStartMenuCursor + 1));
			return true;
		}
	}

	if (serialMode == ReaderSerialMode::ReaderJumpPageInput)
	{
		if (nav == SerialNavInput::Left)
		{
			showReaderStartMenu();
			return true;
		}
		if (nav == SerialNavInput::Up || nav == SerialNavInput::Down || nav == SerialNavInput::Right)
		{
			Serial.println("Jump-to-page still expects a page number for now.");
			Serial.print("> ");
			return true;
		}
	}

	if (serialMode == ReaderSerialMode::ReaderMenu)
	{
		if (nav == SerialNavInput::Up)
		{
			if (!readerShowingCover)
			{
				goToReaderPage(static_cast<int32_t>(readerPageIndex) - 1);
			}
			return true;
		}
		if (nav == SerialNavInput::Down)
		{
			if (!readerShowingCover)
			{
				goToReaderPage(static_cast<int32_t>(readerPageIndex) + 1);
			}
			return true;
		}
		if (nav == SerialNavInput::Left)
		{
			if (readerShowingCover)
			{
				clearReaderState();
				showMainMenu();
			}
			else if (readerHasCover)
			{
				readerCoverReturnPageIndex = readerPageIndex;
				readerShowingCover = true;
				showReaderCoverOnDisplay();
				saveCurrentReaderProgress();
				printReaderHelpToSerial();
			}
			else
			{
				clearReaderState();
				showMainMenu();
			}
			return true;
		}
		if (nav == SerialNavInput::Right)
		{
			if (readerShowingCover)
			{
				Serial.println("[NAV] Exiting cover, going to first page");
				readerShowingCover = false;
				goToReaderPage(static_cast<int32_t>(readerCoverReturnPageIndex));
				return true;
			}
			printReaderHelpToSerial();
			return true;
		}
	}

	return false;
}

void handleSerialInput()
{
	while (Serial.available() > 0)
	{
		const char ch = static_cast<char>(Serial.read());
		if (ch == '\r')
		{
			continue;
		}
		if (ch == '\n')
		{
			if (serialLineBuffer.length() > 0)
			{
				const String input = serialLineBuffer;
				serialLineBuffer = "";

				if (handleSerialNavInput(parseSerialNavInput(input)))
				{
					continue;
				}

				const int value = input.toInt();
				if (value > 0)
				{
					if (serialMode == ReaderSerialMode::DeleteMenu)
					{
						handleDeleteSelection(static_cast<uint16_t>(value));
					}
					else if (serialMode == ReaderSerialMode::SettingsMenu)
					{
						handleSettingsSelection(static_cast<uint16_t>(value));
					}
					else if (serialMode == ReaderSerialMode::ReaderStartMenu)
					{
						handleReaderStartSelection(static_cast<uint16_t>(value));
					}
					else if (serialMode == ReaderSerialMode::ReaderJumpPageInput)
					{
						handleReaderJumpPageSelection(static_cast<uint16_t>(value));
					}
					else if (serialMode == ReaderSerialMode::ReaderMenu)
					{
						handleReaderSelection(static_cast<uint16_t>(value));
					}
					else
					{
						handleMenuSelection(static_cast<uint16_t>(value));
					}
				}
				else
				{
					Serial.println("Use up/down/left/right");
					showInvalidOptionOnDisplay();
					delay(800);
					if (serialMode == ReaderSerialMode::DeleteMenu)
					{
						showDeleteMenu();
					}
					else if (serialMode == ReaderSerialMode::SettingsMenu)
					{
						showSettingsMenu();
					}
					else if (serialMode == ReaderSerialMode::ReaderStartMenu)
					{
						showReaderStartMenu();
					}
					else if (serialMode == ReaderSerialMode::ReaderJumpPageInput)
					{
						promptReaderJumpToPage();
					}
					else if (serialMode == ReaderSerialMode::ReaderMenu)
					{
						printReaderHelpToSerial();
					}
					else
					{
						showMainMenu();
					}
				}
			}
			continue;
		}
		serialLineBuffer += ch;
	}
}
}

void setup()
{
	Serial.begin(115200);
	delay(200);

	if (!LittleFS.begin(true))
	{
		Serial.println("LittleFS mount failed");
	}

	loadReaderSettings();
	loadReaderBookmarks();

	#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
	hspi.begin(13, 12, 14, 15);
	display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
	#endif

	display.init(115200);
	display.setTextWrap(false);
	showMainMenu();
}

void loop()
{
	if (wifiServerRunning)
	{
		server.handleClient();
	}
	else
	{
		handleSerialInput();
	}

	if (shutdownRequested)
	{
		shutdownRequested = false;
		stopWifiServer();
		showMainMenu();
	}

	flushReaderBookmarksToStorage(false);

	delay(2);
}
