#define BITMAP_FILE_TYPE		0x424D
#define BITMAP_HEADER_SIZE		14
#define BITMAP_INFO_HEADER_SIZE	40
#define BITMAP_PALETTE_SIZE		(256*4)
#define COLOR_PLANES			1
#define COLOR_DEPTH				8
#define COMPRESSION_METHOD		0
#define HORIZONTAL_RESOLUTION	0
#define VERTICAL_RESOLUTION		0
#define COLOR_PALETTE			0
#define IMPORTANT_COLORS		0

#define COLOR_BLACK	0
#define COLOR_WHITE	255

void errorMessage(char *err);

void writeHeader(int fd, int32_t width, int32_t height);
void writeInfoHeader(int fd, int32_t width, int32_t height);
void writeColorPalette(int fd);
void writeImage(int fd, uint8_t **pixel_data, int32_t width, int32_t height);