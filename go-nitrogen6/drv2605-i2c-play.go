package main

import (
	"flag"
        "fmt"
        "os/exec"
	"strconv"
)

const DRV2605_BUS=2
const DRV2605_ADDR=0x5A

const DRV2605_REG_STATUS=0x00
const DRV2605_REG_MODE=0x01
const DRV2605_MODE_INTTRIG=0x00
const DRV2605_MODE_EXTTRIGEDGE=0x01
const DRV2605_MODE_EXTTRIGLVL=0x02
const DRV2605_MODE_PWMANALOG=0x03
const DRV2605_MODE_AUDIOVIBE=0x04
const DRV2605_MODE_REALTIME=0x05
const DRV2605_MODE_DIAGNOS=0x06
const DRV2605_MODE_AUTOCAL=0x07

const DRV2605_REG_RTPIN=0x02
const DRV2605_REG_LIBRARY=0x03
const DRV2605_REG_WAVESEQ1=0x04
const DRV2605_REG_WAVESEQ2=0x05
const DRV2605_REG_WAVESEQ3=0x06
const DRV2605_REG_WAVESEQ4=0x07
const DRV2605_REG_WAVESEQ5=0x08
const DRV2605_REG_WAVESEQ6=0x09
const DRV2605_REG_WAVESEQ7=0x0A
const DRV2605_REG_WAVESEQ8=0x0B

const DRV2605_REG_GO=0x0C
const DRV2605_REG_OVERDRIVE=0x0D
const DRV2605_REG_SUSTAINPOS=0x0E
const DRV2605_REG_SUSTAINNEG=0x0F
const DRV2605_REG_BREAK=0x10
const DRV2605_REG_AUDIOCTRL=0x11
const DRV2605_REG_AUDIOLVL=0x12
const DRV2605_REG_AUDIOMAX=0x13
const DRV2605_REG_RATEDV=0x16
const DRV2605_REG_CLAMPV=0x17
const DRV2605_REG_AUTOCALCOMP=0x18
const DRV2605_REG_AUTOCALEMP=0x19
const DRV2605_REG_FEEDBACK=0x1A
const DRV2605_REG_CONTROL1=0x1B
const DRV2605_REG_CONTROL2=0x1C
const DRV2605_REG_CONTROL3=0x1D
const DRV2605_REG_CONTROL4=0x1E
const DRV2605_REG_VBAT=0x21
const DRV2605_REG_LRARESON=0x22

func int2hex(val int) string {
	return "0x" + strconv.FormatInt(int64(val), 16)
}

func hex2int(val []byte) int {
	// it's typically of the form 0xHH\n
	end := len(val) - 1
	num := []byte{}
	if val[end] == '\n' {
		num = val[2:end]
	} else {
		num = val[2:end + 1]
	}
	ret, err := strconv.ParseInt(string(num), 16, 32)
	if err != nil {
		fmt.Printf("%s", err)
	}	
	return int(ret)
}

func writeRegister8(reg int, val int) {
	cmd := exec.Command("i2cset", "-y", int2hex(DRV2605_BUS), int2hex(DRV2605_ADDR), int2hex(reg), int2hex(val))
	//fmt.Println(cmd.Args)
	_, err := cmd.Output()
	if err != nil {
		fmt.Printf("%s", err)
	} 
}

func readRegister8(reg int) int {
	cmd := exec.Command("i2cget", "-y", int2hex(DRV2605_BUS), int2hex(DRV2605_ADDR), int2hex(reg))
	//fmt.Println(cmd.Args)
	out, err := cmd.Output()
	if err != nil {
		fmt.Printf("%s", err)
	}
	//fmt.Println(out[:])
	//fmt.Println(hex2int(out))
	return hex2int(out)
}

func drv2605Init() {
	writeRegister8(DRV2605_REG_MODE, 0x00) // out of standby
	  
	writeRegister8(DRV2605_REG_RTPIN, 0x00); // no real-time-playback
	  
	writeRegister8(DRV2605_REG_WAVESEQ1, 1); // strong click
	writeRegister8(DRV2605_REG_WAVESEQ2, 0);
	  
	writeRegister8(DRV2605_REG_OVERDRIVE, 0); // no overdrive
	  
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

func drv2605SelectLibrary(lib int) {
	writeRegister8(DRV2605_REG_LIBRARY, lib)
}

func drv2605SetMode(mode int) {
	writeRegister8(DRV2605_REG_MODE, mode)
}

func drv2605SetWaveform(slot int, w int) {
	writeRegister8(DRV2605_REG_WAVESEQ1+slot, w)
}

func drv2605Go() {
	writeRegister8(DRV2605_REG_GO, 1);
}

func main() {
	var effect = flag.Int("w", 1, "which stored waveform to play")
	flag.Parse()
	fmt.Printf("playing effect %d\n", *effect)

	drv2605Init()
	drv2605SelectLibrary(1)
	
        // I2C trigger by sending 'go' command
        // default, internal trigger when sending GO command
        drv2605SetMode(DRV2605_MODE_INTTRIG)
	
	drv2605SetWaveform(0, *effect)
	drv2605SetWaveform(1, 0)
	drv2605Go()
}
