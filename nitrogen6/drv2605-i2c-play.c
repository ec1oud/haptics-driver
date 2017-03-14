#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>

#define DRV2605_DEV "/dev/i2c-2"

const int DRV2605_BUS=2;
const int DRV2605_ADDR=0x5A;

const int DRV2605_REG_STATUS=0x00;
const int DRV2605_REG_MODE=0x01;
const int DRV2605_MODE_INTTRIG=0x00;
const int DRV2605_MODE_EXTTRIGEDGE=0x01;
const int DRV2605_MODE_EXTTRIGLVL=0x02;
const int DRV2605_MODE_PWMANALOG=0x03;
const int DRV2605_MODE_AUDIOVIBE=0x04;
const int DRV2605_MODE_REALTIME=0x05;
const int DRV2605_MODE_DIAGNOS=0x06;
const int DRV2605_MODE_AUTOCAL=0x07;

const int DRV2605_REG_RTPIN=0x02;
const int DRV2605_REG_LIBRARY=0x03;
const int DRV2605_REG_WAVESEQ1=0x04;
const int DRV2605_REG_WAVESEQ2=0x05;
const int DRV2605_REG_WAVESEQ3=0x06;
const int DRV2605_REG_WAVESEQ4=0x07;
const int DRV2605_REG_WAVESEQ5=0x08;
const int DRV2605_REG_WAVESEQ6=0x09;
const int DRV2605_REG_WAVESEQ7=0x0A;
const int DRV2605_REG_WAVESEQ8=0x0B;

const int DRV2605_REG_GO=0x0C;
const int DRV2605_REG_OVERDRIVE=0x0D;
const int DRV2605_REG_SUSTAINPOS=0x0E;
const int DRV2605_REG_SUSTAINNEG=0x0F;
const int DRV2605_REG_BREAK=0x10;
const int DRV2605_REG_AUDIOCTRL=0x11;
const int DRV2605_REG_AUDIOLVL=0x12;
const int DRV2605_REG_AUDIOMAX=0x13;
const int DRV2605_REG_RATEDV=0x16;
const int DRV2605_REG_CLAMPV=0x17;
const int DRV2605_REG_AUTOCALCOMP=0x18;
const int DRV2605_REG_AUTOCALEMP=0x19;
const int DRV2605_REG_FEEDBACK=0x1A;
const int DRV2605_REG_CONTROL1=0x1B;
const int DRV2605_REG_CONTROL2=0x1C;
const int DRV2605_REG_CONTROL3=0x1D;
const int DRV2605_REG_CONTROL4=0x1E;
const int DRV2605_REG_VBAT=0x21;
const int DRV2605_REG_LRARESON=0x22;

int devFile = 0;

void writeRegister8(char reg, char val)
{
	/* Using I2C Write, equivalent of
	i2c_smbus_write_word_data(file, reg, 0x6543) */
	char buf[2];
	buf[0] = reg;
	buf[1] = val;
	if (write(devFile, buf, 2) != 2) {
		perror("i2c write failed");
	}
}

char readRegister8(char reg)
{
	char ret = 0;
	/* Using I2C Read, equivalent of i2c_smbus_read_byte(file) */
	if (read(devFile, &ret, 1) != 1) {
		perror("i2c read failed");
		ret = 0;
	}
	return ret;
}

void drv2605Init()
{
	devFile = open(DRV2605_DEV, O_RDWR);
	if (devFile < 0) {
		perror("failed to open " DRV2605_DEV);
		exit(1);
	}

	if (ioctl(devFile, I2C_SLAVE, DRV2605_ADDR) < 0) {
		perror("failed to select device");
		exit(2);
	}

	writeRegister8(DRV2605_REG_MODE, 0x00);		  // out of standby

	writeRegister8(DRV2605_REG_RTPIN, 0x00);	  // no real-time-playback

	writeRegister8(DRV2605_REG_WAVESEQ1, 1);	  // strong click
	writeRegister8(DRV2605_REG_WAVESEQ2, 0);

	writeRegister8(DRV2605_REG_OVERDRIVE, 0);	  // no overdrive

	writeRegister8(DRV2605_REG_SUSTAINPOS, 0);
	writeRegister8(DRV2605_REG_SUSTAINNEG, 0);
	writeRegister8(DRV2605_REG_BREAK, 0);
	writeRegister8(DRV2605_REG_AUDIOMAX, 0x64);

	// ERM open loop

	// turn off N_ERM_LRA
	writeRegister8(DRV2605_REG_FEEDBACK, readRegister8(DRV2605_REG_FEEDBACK) & 0x7F);
	// turn on ERM_OPEN_LOOP
	writeRegister8(DRV2605_REG_CONTROL3, readRegister8(DRV2605_REG_CONTROL3) | 0x20);
}

void drv2605SelectLibrary(int lib)
{
	writeRegister8(DRV2605_REG_LIBRARY, lib);
}

void drv2605SetMode(int mode)
{
	writeRegister8(DRV2605_REG_MODE, mode);
}

void drv2605SetWaveform(int slot, int w)
{
	writeRegister8(DRV2605_REG_WAVESEQ1+slot, w);
}

void drv2605Go()
{
	writeRegister8(DRV2605_REG_GO, 1);
}

int main(int argc, char** argv)
{
	int effect = 1;
	if (argc < 2) {
		fprintf(stderr, "Usage: %s effect-number\n", argv[0]);
		return -1;
	}
	effect = atoi(argv[1]);
	printf("playing effect %d\n", effect);

	drv2605Init();
	drv2605SelectLibrary(1);

	// I2C trigger by sending 'go' command
	// default, internal trigger when sending GO command
	drv2605SetMode(DRV2605_MODE_INTTRIG);

	drv2605SetWaveform(0, effect);
	drv2605SetWaveform(1, 0);
	drv2605Go();
}
