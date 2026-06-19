#ifndef SAVELOAD_NOCLICK_H
#define SAVELOAD_NOCLICK_H

/* Shared noclick SD helpers — code in SRAM3 (.sram3.text.slnc*) to save SRAM1.
 * Prefix attribute form required: C++ rejects postfix attrs on global definitions. */
#define SAVELOAD_NOCLICK_SRAM3_FN(n) __attribute__((noinline, section(".sram3.text.slnc" #n)))

static bool_t saveload_noclick_busy = false;

SAVELOAD_NOCLICK_SRAM3_FN(1)
static void wait_busy(void) {
	volatile uint32_t count = 600000; /* 10 minutes */
	while (saveload_noclick_busy && count) {
		count--;
		chThdSleepMilliseconds(1);
	}
}

SAVELOAD_NOCLICK_SRAM3_FN(2)
static void save_sd(const char* fname, int32_t* array, uint32_t LENGTH) {

	wait_busy();
	saveload_noclick_busy = true;

	FIL FileObject;
	FRESULT err;
	UINT bytes_written;

	err = f_open(&FileObject, fname, FA_WRITE | FA_CREATE_ALWAYS);
	if (err != FR_OK) {
		report_fatfs_error(err, fname);
		saveload_noclick_busy = false;
		return;
	}

	int rem_sz = (int)(sizeof(int32_t) * LENGTH);
	int offset = 0;

	while (rem_sz > 0) {
		if (rem_sz > (int)sizeof(fbuff)) {
			memcpy((char*) fbuff, (char*) array + offset, sizeof(fbuff));
			err = f_write(&FileObject, fbuff, sizeof(fbuff), &bytes_written);
			rem_sz -= (int)sizeof(fbuff);
			offset += (int)sizeof(fbuff);
		} else {
			memcpy((char*) fbuff, (char*) array + offset, rem_sz);
			err = f_write(&FileObject, fbuff, rem_sz, &bytes_written);
			rem_sz = 0;
		}
	}
	if (err != FR_OK) {
		report_fatfs_error(err, fname);
		saveload_noclick_busy = false;
		return;
	}

	err = f_close(&FileObject);
	if (err != FR_OK) {
		report_fatfs_error(err, fname);
		saveload_noclick_busy = false;
		return;
	}
	saveload_noclick_busy = false;
}


SAVELOAD_NOCLICK_SRAM3_FN(3)
static void load_sd(const char* fname, int32_t* array, uint32_t LENGTH) {

	wait_busy();
	saveload_noclick_busy = true;

	FIL FileObject;
	FRESULT err;
	UINT bytes_read;

	err = f_open(&FileObject, fname, FA_READ | FA_OPEN_EXISTING);
	if (err != FR_OK) {
		report_fatfs_error(err, fname);
		saveload_noclick_busy = false;
		return;
	}

	int rem_sz = (int)(sizeof(int32_t) * LENGTH);
	int offset = 0;

	while (rem_sz > 0) {
		if (rem_sz > (int)sizeof(fbuff)) {
			err = f_read(&FileObject, fbuff, sizeof(fbuff), &bytes_read);
			if (bytes_read == 0) {
				break;
			}
			memcpy((char*) array + offset, (char*) fbuff, bytes_read);
			rem_sz -= (int)bytes_read;
			offset += (int)bytes_read;
		} else {
			err = f_read(&FileObject, fbuff, rem_sz, &bytes_read);
			memcpy((char*) array + offset, (char*) fbuff, bytes_read);
			rem_sz = 0;
		}
	}
	if (err != FR_OK) {
		report_fatfs_error(err, fname);
		saveload_noclick_busy = false;
		return;
	}

	err = f_close(&FileObject);
	if (err != FR_OK) {
		report_fatfs_error(err, fname);
		saveload_noclick_busy = false;
		return;
	}
	saveload_noclick_busy = false;
}


#endif
