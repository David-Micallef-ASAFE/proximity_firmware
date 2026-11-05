/**
 * @file      dw3000_direct.h
 *
 * @brief     Direct (non-virtualised) entry points into the DW3000 low-level driver.
 *
 *            Each function here corresponds 1:1 with one `case` of the former
 *            dwt_ioctl() dispatch switch.  Calling them by name instead of through
 *            dwt_ops::ioctl() means the linker sees exactly which parts of the driver
 *            the application actually uses, and --gc-sections can discard the rest.
 *
 *            Generated from the DWT_* cases of dwt_ioctl() in dw3000_device.c.
 *
 * @copyright SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo US, Inc.
 *            SPDX-License-Identifier: LicenseRef-QORVO-2
 */
#ifndef DW3000_DIRECT_H
#define DW3000_DIRECT_H

#include "deca_device_api.h"
#include "deca_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Operations that used to be reached through struct dwt_ops_s / dwt_mcps_ops_s.
 * These are the real driver functions, no longer static.
 * ------------------------------------------------------------------------- */
int32_t ull_initialise(dwchip_t *dw, int32_t mode);
int32_t ull_configure(dwchip_t *dw, dwt_config_t *config);
void ull_configuretxrf(dwchip_t *dw, dwt_txconfig_t *config);
int32_t ull_writetxdata(dwchip_t *dw, uint16_t txDataLength, uint8_t *txDataBytes, uint16_t txBufferOffset);
void ull_writetxfctrl(dwchip_t *dw, uint16_t txFrameLength, uint16_t txBufferOffset, uint8_t ranging);
void ull_readrxdata(dwchip_t *dw, uint8_t *buffer, uint16_t length, uint16_t rxBufferOffset);
void ull_readaccdata(dwchip_t *dw, uint8_t *buffer, uint16_t length, uint16_t accOffset);
void ull_readcir(dwchip_t *dw, uint32_t *buffer, dwt_acc_idx_e cir_idx, uint16_t sample_offs,
                 uint16_t num_samples, dwt_cir_read_mode_e mode);
void ull_readrxtimestamp(dwchip_t *dw, uint8_t *timestamp);
void ull_setinterrupt(dwchip_t *dw, uint32_t bitmask_lo, uint32_t bitmask_hi, dwt_INT_options_e INT_options);
int32_t ull_rxenable(dwchip_t *dw, int32_t mode);
void ull_isr(dwchip_t *dw);
int32_t ull_setchannel(dwchip_t *dw, uint8_t ch);
void ull_writetodevice(dwchip_t *dw, uint32_t regFileID, uint16_t index, uint16_t length, uint8_t *buffer);
void ull_readfromdevice(dwchip_t *dw, uint32_t regFileID, uint16_t index, uint16_t length, uint8_t *buffer);
void *dwt_dbg_fn(dwchip_t *dw, dwt_ioctl_e fn, int32_t parm, void *ptr);

/* ---------------------------------------------------------------------------
 * One entry point per former ioctl() command.
 * Signature mirrors the old ioctl(): (dw, parm, ptr) -> int32_t status.
 * ------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Commands that exist in the API but are implemented only by the DW3720.
 * The old dwt_ioctl() answered these with its `default:` case; this keeps that
 * exact behaviour without a dispatcher.
 * ------------------------------------------------------------------------- */
static inline int32_t dw3000_ioc_unsupported(dwchip_t *dw, int32_t parm, void *ptr)
{
    (void)dw;
    (void)parm;
    (void)ptr;
    return -1; /* _Err_not_supported, as per the old ioctl() default case */
}

int32_t dw3000_ioc_wakeup(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_WAKEUP */
int32_t dw3000_ioc_forcetrxoff(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_FORCETRXOFF */
int32_t dw3000_ioc_starttx(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_STARTTX */
int32_t dw3000_ioc_setdelayedtrxtime(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETDELAYEDTRXTIME */
int32_t dw3000_ioc_setkeyreg128(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETKEYREG128 */
int32_t dw3000_ioc_configureleaddress(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGURELEADDRESS */
int32_t dw3000_ioc_set_txpower(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SET_TXPOWER */
int32_t dw3000_ioc_configuresfdtype(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGURESFDTYPE */
int32_t dw3000_ioc_enablegpioclocks(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_ENABLEGPIOCLOCKS */
int32_t dw3000_ioc_otprevision(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_OTPREVISION */
int32_t dw3000_ioc_geticrefvolt(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GETICREFVOLT */
int32_t dw3000_ioc_geticreftemp(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GETICREFTEMP */
int32_t dw3000_ioc_getpartid(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GETPARTID */
int32_t dw3000_ioc_getlotid(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GETLOTID */
int32_t dw3000_ioc_signalrxbufffree(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SIGNALRXBUFFFREE */
int32_t dw3000_ioc_setrxaftertxdelay(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETRXAFTERTXDELAY */
int32_t dw3000_ioc_enablespicrccheck(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_ENABLESPICRCCHECK */
int32_t dw3000_ioc_enableautoack(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_ENABLEAUTOACK */
int32_t dw3000_ioc_checkdevid(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CHECKDEVID */
int32_t dw3000_ioc_configciadiag(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGCIADIAG */
int32_t dw3000_ioc_entersleepaftertx(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_ENTERSLEEPAFTERTX */
int32_t dw3000_ioc_entersleepafter(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_ENTERSLEEPAFTER */
int32_t dw3000_ioc_setfinegraintxseq(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETFINEGRAINTXSEQ */
int32_t dw3000_ioc_setlnapamode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETLNAPAMODE */
int32_t dw3000_ioc_readpgdelay(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READPGDELAY */
int32_t dw3000_ioc_configurestskey(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGURESTSKEY */
int32_t dw3000_ioc_configurestsiv(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGURESTSIV */
int32_t dw3000_ioc_configurestsloadiv(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGURESTSLOADIV */
int32_t dw3000_ioc_configmrxlut(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGMRXLUT */
int32_t dw3000_ioc_restoreconfig(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_RESTORECONFIG */
int32_t dw3000_ioc_configurestsmode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGURESTSMODE */
int32_t dw3000_ioc_setrxantennadelay(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETRXANTENNADELAY */
int32_t dw3000_ioc_getrxantennadelay(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GETRXANTENNADELAY */
int32_t dw3000_ioc_settxantennadelay(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETTXANTENNADELAY */
int32_t dw3000_ioc_gettxantennadelay(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GETTXANTENNADELAY */
int32_t dw3000_ioc_writerxscratchdata(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_WRITERXSCRATCHDATA */
int32_t dw3000_ioc_readrxscratchdata(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READRXSCRATCHDATA */
int32_t dw3000_ioc_readrxdata(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READRXDATA */
int32_t dw3000_ioc_writetxdata(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_WRITETXDATA */
int32_t dw3000_ioc_rxenable(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_RXENABLE */
int32_t dw3000_ioc_writetxfctrl(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_WRITETXFCTRL */
int32_t dw3000_ioc_readclockoffset(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READCLOCKOFFSET */
int32_t dw3000_ioc_readcarrierintegrator(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READCARRIERINTEGRATOR */
int32_t dw3000_ioc_clearaonconfig(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CLEARAONCONFIG */
int32_t dw3000_ioc_calcbandwidthadj(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CALCBANDWIDTHADJ */
int32_t dw3000_ioc_readdiagnostics(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READDIAGNOSTICS */
int32_t dw3000_ioc_readdiagnostics_acc(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READDIAGNOSTICS_ACC */
int32_t dw3000_ioc_readtxtimestamphi32(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READTXTIMESTAMPHI32 */
int32_t dw3000_ioc_readtxtimestamplo32(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READTXTIMESTAMPLO32 */
int32_t dw3000_ioc_readtxtimestamp(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READTXTIMESTAMP */
int32_t dw3000_ioc_readpdoa(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READPDOA */
int32_t dw3000_ioc_readtdoa(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READTDOA */
int32_t dw3000_ioc_readwakeuptemp(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READWAKEUPTEMP */
int32_t dw3000_ioc_readwakeupvbat(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READWAKEUPVBAT */
int32_t dw3000_ioc_otpwrite(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_OTPWRITE */
int32_t dw3000_ioc_otpwriteandverify(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_OTPWRITEANDVERIFY */
int32_t dw3000_ioc_entersleep(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_ENTERSLEEP */
int32_t dw3000_ioc_configuresleepcnt(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGURESLEEPCNT */
int32_t dw3000_ioc_calibratesleepcnt(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CALIBRATESLEEPCNT */
int32_t dw3000_ioc_configuresleep(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGURESLEEP */
int32_t dw3000_ioc_softreset(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SOFTRESET */
int32_t dw3000_ioc_setxtaltrim(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETXTALTRIM */
int32_t dw3000_ioc_getxtaltrim(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GETXTALTRIM */
int32_t dw3000_ioc_configcwmode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGCWMODE */
int32_t dw3000_ioc_repeatedcw(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_REPEATEDCW */
int32_t dw3000_ioc_readtempvbat(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READTEMPVBAT */
int32_t dw3000_ioc_convertrawtemp(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONVERTRAWTEMP */
int32_t dw3000_ioc_convertrawvbat(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONVERTRAWVBAT */
int32_t dw3000_ioc_configcontinuousframemode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGCONTINUOUSFRAMEMODE */
int32_t dw3000_ioc_disablecontinuousframemode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_DISABLECONTINUOUSFRAMEMODE */
int32_t dw3000_ioc_disablecontinuouswavemode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_DISABLECONTINUOUSWAVEMODE */
int32_t dw3000_ioc_stoprepeatedframes(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_STOPREPEATEDFRAMES */
int32_t dw3000_ioc_repeatedpreamble(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_REPEATEDPREAMBLE */
int32_t dw3000_ioc_repeatedframes(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_REPEATEDFRAMES */
int32_t dw3000_ioc_readstsquality(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READSTSQUALITY */
int32_t dw3000_ioc_doaes(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_DOAES */
int32_t dw3000_ioc_configureaes(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGUREAES */
int32_t dw3000_ioc_micsizefrombytes(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_MICSIZEFROMBYTES */
int32_t dw3000_ioc_readeventcounters(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READEVENTCOUNTERS */
int32_t dw3000_ioc_configeventcounters(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGEVENTCOUNTERS */
int32_t dw3000_ioc_setpreambledetecttimeout(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETPREAMBLEDETECTTIMEOUT */
int32_t dw3000_ioc_setsniffmode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETSNIFFMODE */
int32_t dw3000_ioc_setrxtimeout(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETRXTIMEOUT */
int32_t dw3000_ioc_aonread(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_AONREAD */
int32_t dw3000_ioc_aonwrite(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_AONWRITE */
int32_t dw3000_ioc_readstsstatus(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READSTSSTATUS */
int32_t dw3000_ioc_setleds(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETLEDS */
int32_t dw3000_ioc_setdwstate(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETDWSTATE */
int32_t dw3000_ioc_readsystime(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READSYSTIME */
int32_t dw3000_ioc_checkidlerc(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CHECKIDLERC */
int32_t dw3000_ioc_checkirq(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CHECKIRQ */
int32_t dw3000_ioc_configureframefilter(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGUREFRAMEFILTER */
int32_t dw3000_ioc_seteui(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETEUI */
int32_t dw3000_ioc_geteui(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GETEUI */
int32_t dw3000_ioc_setpanid(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETPANID */
int32_t dw3000_ioc_setaddress16(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETADDRESS16 */
int32_t dw3000_ioc_readrxtimestamp(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READRXTIMESTAMP */
int32_t dw3000_ioc_readrxtimestamp_ipatov(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READRXTIMESTAMP_IPATOV */
int32_t dw3000_ioc_readrxtimestampunadj(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READRXTIMESTAMPUNADJ */
int32_t dw3000_ioc_readrxtimestamphi32(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READRXTIMESTAMPHI32 */
int32_t dw3000_ioc_readrxtimestamplo32(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READRXTIMESTAMPLO32 */
int32_t dw3000_ioc_readrxtimestamp_sts(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READRXTIMESTAMP_STS */
int32_t dw3000_ioc_readsystimestamphi32(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READSYSTIMESTAMPHI32 */
int32_t dw3000_ioc_otpread(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_OTPREAD */
int32_t dw3000_ioc_setplenfine(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETPLENFINE */
int32_t dw3000_ioc_runpgfcal(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_RUNPGFCAL */
int32_t dw3000_ioc_pgf_cal(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_PGF_CAL */
int32_t dw3000_ioc_calcpgcount(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CALCPGCOUNT */
int32_t dw3000_ioc_pll_status(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_PLL_STATUS */
int32_t dw3000_ioc_pll_cal(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_PLL_CAL */
int32_t dw3000_ioc_configure_rf_port(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONFIGURE_RF_PORT */
int32_t dw3000_ioc_setgpiomode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETGPIOMODE */
int32_t dw3000_ioc_setgpiodir(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETGPIODIR */
int32_t dw3000_ioc_setgpiovalue(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETGPIOVALUE */
int32_t dw3000_ioc_setdblrxbuffmode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETDBLRXBUFFMODE */
int32_t dw3000_ioc_setreferencetrxtime(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETREFERENCETRXTIME */
int32_t dw3000_ioc_read_reg(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READ_REG */
int32_t dw3000_ioc_write_reg(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_WRITE_REG */
int32_t dw3000_ioc_getdgcdecision(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GETDGCDECISION */
#ifdef WIN32
int32_t dw3000_ioc_spicswakeup(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SPICSWAKEUP */
#endif
int32_t dw3000_ioc_writesysstatuslo(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_WRITESYSSTATUSLO */
int32_t dw3000_ioc_writesysstatushi(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_WRITESYSSTATUSHI */
int32_t dw3000_ioc_readsysstatuslo(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READSYSSTATUSLO */
int32_t dw3000_ioc_readsysstatushi(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READSYSSTATUSHI */
int32_t dw3000_ioc_writerdbstatus(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_WRITERDBSTATUS */
int32_t dw3000_ioc_readrdbstatus(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READRDBSTATUS */
int32_t dw3000_ioc_getframelength(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GETFRAMELENGTH */
int32_t dw3000_ioc_readgpiovalue(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READGPIOVALUE */
int32_t dw3000_ioc_readpdoaoffset(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READPDOAOFFSET */
int32_t dw3000_ioc_setpdoaoffset(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETPDOAOFFSET */
int32_t dw3000_ioc_adj_txpower(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_ADJ_TXPOWER */
int32_t dw3000_ioc_linear_txpower(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_LINEAR_TXPOWER */
int32_t dw3000_ioc_convert_txpower_to_idx(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CONVERT_TXPOWER_TO_IDX */
int32_t dw3000_ioc_set_pll_config(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SET_PLL_CONFIG */
int32_t dw3000_ioc_cfgwificoexset(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CFGWIFICOEXSET */
int32_t dw3000_ioc_cfgantsel(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CFGANTSEL */
int32_t dw3000_ioc_rstsystemcnt(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_RSTSYSTEMCNT */
int32_t dw3000_ioc_cfgostrmode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CFGOSTRMODE */
int32_t dw3000_ioc_pll_auto_cal(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_PLL_AUTO_CAL */
int32_t dw3000_ioc_set_sts_len(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SET_STS_LEN */
int32_t dw3000_ioc_cfg_sts(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CFG_STS */
int32_t dw3000_ioc_setpdoamode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SETPDOAMODE */
int32_t dw3000_ioc_set_fcs_mode(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SET_FCS_MODE */
int32_t dw3000_ioc_set_phr(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SET_PHR */
int32_t dw3000_ioc_set_datarate(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SET_DATARATE */
int32_t dw3000_ioc_set_pac(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SET_PAC */
int32_t dw3000_ioc_set_sfdto(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SET_SFDTO */
int32_t dw3000_ioc_readdgcdbg(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READDGCDBG */
int32_t dw3000_ioc_readctrdbg(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_READCTRDBG */
int32_t dw3000_ioc_get_cir_regadd(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_GET_CIR_REGADD */
int32_t dw3000_ioc_dbg_regs(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_DBG_REGS */
int32_t dw3000_ioc_cia_version(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CIA_VERSION */
int32_t dw3000_ioc_nlos_ipdiag(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_NLOS_IPDIAG */
int32_t dw3000_ioc_nlos_alldiag(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_NLOS_ALLDIAG */
int32_t dw3000_ioc_dis_otp_ips(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_DIS_OTP_IPS */
int32_t dw3000_ioc_calculate_rssi(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CALCULATE_RSSI */
int32_t dw3000_ioc_calculate_first_path_power(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_CALCULATE_FIRST_PATH_POWER */
int32_t dw3000_ioc_set_isr_flags(dwchip_t *dw, int32_t parm, void *ptr); /* was DWT_SET_ISR_FLAGS */

#ifdef __cplusplus
}
#endif

#endif /* DW3000_DIRECT_H */
