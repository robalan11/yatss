#define BLOCK_SIZE 8192
#define BLOCK_COUNT 20
#define SAMPLE_RATE 44100
#define SR SAMPLE_RATE
#define MAX_AMP 32767
#define CHANNELS 1
#define M_PI 3.1415926

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define clamp(num,min_val,max_val) max(min((num),(max_val)),(min_val))