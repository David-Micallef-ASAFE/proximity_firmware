/**
 * @file      dw3000_direct.inc
 *
 * @brief     Bodies of the direct entry points declared in dw3000_direct.h.
 *
 *            Included at the end of dw3000_device.c because the ull_*() functions
 *            it calls have internal linkage.  Each function below is the verbatim
 *            body of the matching `case` of the original dwt_ioctl() switch, wrapped
 *            in do{}while(false) so that any `break` keeps its original meaning.
 *
 *            Generated - do not edit by hand.
 *
 * @copyright SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo US, Inc.
 *            SPDX-License-Identifier: LicenseRef-QORVO-2
 */

/* DWT_WAKEUP */
int32_t dw3000_ioc_wakeup(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_wakeup_ic(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_FORCETRXOFF */
int32_t dw3000_ioc_forcetrxoff(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_forcetrxoff(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_STARTTX */
int32_t dw3000_ioc_starttx(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            ret = ull_starttx(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETDELAYEDTRXTIME */
int32_t dw3000_ioc_setdelayedtrxtime(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            ull_setdelayedtrxtime(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETKEYREG128 */
int32_t dw3000_ioc_setkeyreg128(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_set_keyreg_128(dw, (const dwt_aes_key_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGURELEADDRESS */
int32_t dw3000_ioc_configureleaddress(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_configure_le_address_s *tmp = (struct dwt_configure_le_address_s *)ptr;
            ull_configure_le_address(dw, tmp->addr, tmp->leIndex);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SET_TXPOWER */
int32_t dw3000_ioc_set_txpower(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            ull_settxpower(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGURESFDTYPE */
int32_t dw3000_ioc_configuresfdtype(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            ull_configuresfdtype(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_ENABLEGPIOCLOCKS */
int32_t dw3000_ioc_enablegpioclocks(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_enablegpioclocks(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_OTPREVISION */
int32_t dw3000_ioc_otprevision(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_otprevision(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_GETICREFVOLT */
int32_t dw3000_ioc_geticrefvolt(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_geticrefvolt(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_GETICREFTEMP */
int32_t dw3000_ioc_geticreftemp(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_geticreftemp(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_GETPARTID */
int32_t dw3000_ioc_getpartid(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = ull_getpartid(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_GETLOTID */
int32_t dw3000_ioc_getlotid(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint64_t *tmp = (uint64_t *)ptr;
            *tmp = ull_getlotid(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SIGNALRXBUFFFREE */
int32_t dw3000_ioc_signalrxbufffree(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_signal_rx_buff_free(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_SETRXAFTERTXDELAY */
int32_t dw3000_ioc_setrxaftertxdelay(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            ull_setrxaftertxdelay(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_ENABLESPICRCCHECK */
int32_t dw3000_ioc_enablespicrccheck(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_enable_spi_crc_check_s *tmp = (struct dwt_enable_spi_crc_check_s *)ptr;
            ull_enablespicrccheck(dw, tmp->crc_mode, tmp->spireaderr_cb);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_ENABLEAUTOACK */
int32_t dw3000_ioc_enableautoack(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_enable_auto_ack_s *tmp = (struct dwt_enable_auto_ack_s *)ptr;
            ull_enableautoack(dw, tmp->responseDelayTime, tmp->enable);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CHECKDEVID */
int32_t dw3000_ioc_checkdevid(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ret = ull_check_dev_id(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGCIADIAG */
int32_t dw3000_ioc_configciadiag(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            ull_configciadiag(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_ENTERSLEEPAFTERTX */
int32_t dw3000_ioc_entersleepaftertx(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_entersleepaftertx(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_ENTERSLEEPAFTER */
int32_t dw3000_ioc_entersleepafter(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_entersleepafter(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_SETFINEGRAINTXSEQ */
int32_t dw3000_ioc_setfinegraintxseq(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_setfinegraintxseq(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_SETLNAPAMODE */
int32_t dw3000_ioc_setlnapamode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_setlnapamode(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_READPGDELAY */
int32_t dw3000_ioc_readpgdelay(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_readpgdelay(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGURESTSKEY */
int32_t dw3000_ioc_configurestskey(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_configurestskey(dw, ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGURESTSIV */
int32_t dw3000_ioc_configurestsiv(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_configurestsiv(dw, ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGURESTSLOADIV */
int32_t dw3000_ioc_configurestsloadiv(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_configurestsloadiv(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGMRXLUT */
int32_t dw3000_ioc_configmrxlut(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_configmrxlut(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_RESTORECONFIG */
int32_t dw3000_ioc_restoreconfig(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_restoreconfig(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGURESTSMODE */
int32_t dw3000_ioc_configurestsmode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            ull_configurestsmode(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETRXANTENNADELAY */
int32_t dw3000_ioc_setrxantennadelay(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            ull_setrxantennadelay(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_GETRXANTENNADELAY */
int32_t dw3000_ioc_getrxantennadelay(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            *tmp = ull_getrxantennadelay(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETTXANTENNADELAY */
int32_t dw3000_ioc_settxantennadelay(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            ull_settxantennadelay(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_GETTXANTENNADELAY */
int32_t dw3000_ioc_gettxantennadelay(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            *tmp = ull_gettxantennadelay(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_WRITERXSCRATCHDATA */
int32_t dw3000_ioc_writerxscratchdata(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_rw_data_s *rd = (struct dwt_rw_data_s *)ptr;
            ull_write_rx_scratch_data(dw, rd->buffer, rd->length, rd->offset);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READRXSCRATCHDATA */
int32_t dw3000_ioc_readrxscratchdata(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_rw_data_s *rd = (struct dwt_rw_data_s *)ptr;
            ull_read_rx_scratch_data(dw, rd->buffer, rd->length, rd->offset);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READRXDATA */
int32_t dw3000_ioc_readrxdata(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_rw_data_s *rd = (struct dwt_rw_data_s *)ptr;
            ull_readrxdata(dw, rd->buffer, rd->length, rd->offset);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_WRITETXDATA */
int32_t dw3000_ioc_writetxdata(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_rw_data_s *wr = (struct dwt_rw_data_s *)ptr;
            (void)ull_writetxdata(dw, wr->length, wr->buffer, wr->offset);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_RXENABLE */
int32_t dw3000_ioc_rxenable(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        (void)ull_rxenable(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_WRITETXFCTRL */
int32_t dw3000_ioc_writetxfctrl(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_tx_fctrl_s *txfctrl = (struct dwt_tx_fctrl_s *)ptr;
            ull_writetxfctrl(dw, txfctrl->txFrameLength, txfctrl->txBufferOffset, txfctrl->ranging);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READCLOCKOFFSET */
int32_t dw3000_ioc_readclockoffset(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            int16_t *tmp = (int16_t *)ptr;
            *tmp = ull_readclockoffset(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READCARRIERINTEGRATOR */
int32_t dw3000_ioc_readcarrierintegrator(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            int32_t *tmp = (int32_t *)ptr;
            *tmp = ull_readcarrierintegrator(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CLEARAONCONFIG */
int32_t dw3000_ioc_clearaonconfig(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_clearaonconfig(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_CALCBANDWIDTHADJ */
int32_t dw3000_ioc_calcbandwidthadj(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_calc_bandwidth_adj_s *tmp = (struct dwt_calc_bandwidth_adj_s *)ptr;
            tmp->result = ull_calcbandwidthadj(dw, tmp->target_count);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READDIAGNOSTICS */
int32_t dw3000_ioc_readdiagnostics(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_readdiagnostics(dw, (dwt_rxdiag_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_READDIAGNOSTICS_ACC */
int32_t dw3000_ioc_readdiagnostics_acc(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_readdiagnostics_acc_s *tmp = (struct dwt_readdiagnostics_acc_s *)ptr;
            ret = ull_readdiagnostics_acc(dw, tmp->cir_diag, tmp->acc_idx);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READTXTIMESTAMPHI32 */
int32_t dw3000_ioc_readtxtimestamphi32(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = ull_readtxtimestamphi32(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READTXTIMESTAMPLO32 */
int32_t dw3000_ioc_readtxtimestamplo32(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = ull_readtxtimestamplo32(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READTXTIMESTAMP */
int32_t dw3000_ioc_readtxtimestamp(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_readtxtimestamp(dw, (uint8_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_READPDOA */
int32_t dw3000_ioc_readpdoa(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            int16_t *tmp = (int16_t *)ptr;
            *tmp = ull_readpdoa(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READTDOA */
int32_t dw3000_ioc_readtdoa(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_readtdoa(dw, (uint8_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_READWAKEUPTEMP */
int32_t dw3000_ioc_readwakeuptemp(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_readwakeuptemp(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READWAKEUPVBAT */
int32_t dw3000_ioc_readwakeupvbat(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_readwakeupvbat(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_OTPWRITE */
int32_t dw3000_ioc_otpwrite(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_opt_write_and_verify_s *tmp = (struct dwt_opt_write_and_verify_s *)ptr;
            ret = ull_otpwrite(dw, tmp->value, tmp->address);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_OTPWRITEANDVERIFY */
int32_t dw3000_ioc_otpwriteandverify(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_opt_write_and_verify_s *tmp = (struct dwt_opt_write_and_verify_s *)ptr;
            ret = ull_otpwriteandverify(dw, tmp->value, tmp->address);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_ENTERSLEEP */
int32_t dw3000_ioc_entersleep(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_entersleep(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGURESLEEPCNT */
int32_t dw3000_ioc_configuresleepcnt(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            ull_configuresleepcnt(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CALIBRATESLEEPCNT */
int32_t dw3000_ioc_calibratesleepcnt(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            *tmp = ull_calibratesleepcnt(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGURESLEEP */
int32_t dw3000_ioc_configuresleep(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_configure_sleep_s *tmp = (struct dwt_configure_sleep_s *)ptr;
            ull_configuresleep(dw, tmp->mode, tmp->wake);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SOFTRESET */
int32_t dw3000_ioc_softreset(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_softreset(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_SETXTALTRIM */
int32_t dw3000_ioc_setxtaltrim(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            ull_setxtaltrim(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_GETXTALTRIM */
int32_t dw3000_ioc_getxtaltrim(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_getxtaltrim(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGCWMODE */
int32_t dw3000_ioc_configcwmode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_configcwmode(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_REPEATEDCW */
int32_t dw3000_ioc_repeatedcw(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_repeated_cw_s *tmp = (struct dwt_repeated_cw_s *)ptr;
            ull_repeated_cw(dw, tmp->cw_enable, tmp->cw_mode_config);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READTEMPVBAT */
int32_t dw3000_ioc_readtempvbat(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            *tmp = ull_readtempvbat(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CONVERTRAWTEMP */
int32_t dw3000_ioc_convertrawtemp(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_convert_raw_temp_s *tmp = (struct dwt_convert_raw_temp_s *)ptr;
            tmp->result = ull_convertrawtemperature(dw, tmp->raw_temp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CONVERTRAWVBAT */
int32_t dw3000_ioc_convertrawvbat(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_convert_raw_volt_s *tmp = (struct dwt_convert_raw_volt_s *)ptr;
            tmp->result = ull_convertrawvoltage(dw, tmp->raw_voltage);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGCONTINUOUSFRAMEMODE */
int32_t dw3000_ioc_configcontinuousframemode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            ull_configcontinuousframemode(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_DISABLECONTINUOUSFRAMEMODE */
int32_t dw3000_ioc_disablecontinuousframemode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_disablecontinuousframemode(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_DISABLECONTINUOUSWAVEMODE */
int32_t dw3000_ioc_disablecontinuouswavemode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_disablecontinuouswavemode(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_STOPREPEATEDFRAMES */
int32_t dw3000_ioc_stoprepeatedframes(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_stop_repeated_frames(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_REPEATEDPREAMBLE */
int32_t dw3000_ioc_repeatedpreamble(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_repeated_p_s *tmp = (struct dwt_repeated_p_s *)ptr;
            ull_send_test_preamble(dw, tmp->delay, tmp->test_txpower);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_REPEATEDFRAMES */
int32_t dw3000_ioc_repeatedframes(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            ull_repeated_frames(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READSTSQUALITY */
int32_t dw3000_ioc_readstsquality(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ret = ull_readstsquality(dw, (int16_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_DOAES */
int32_t dw3000_ioc_doaes(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_do_aes_s *tmp = (struct dwt_do_aes_s *)ptr;
            tmp->result = ull_do_aes(dw, tmp->job, tmp->core_type);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGUREAES */
int32_t dw3000_ioc_configureaes(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_configure_aes(dw, (const dwt_aes_config_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_MICSIZEFROMBYTES */
int32_t dw3000_ioc_micsizefrombytes(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_mic_size_from_bytes_s *tmp = (struct dwt_mic_size_from_bytes_s *)ptr;
            tmp->result = ull_mic_size_from_bytes(dw, tmp->mic_size_in_bytes);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READEVENTCOUNTERS */
int32_t dw3000_ioc_readeventcounters(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_readeventcounters(dw, (dwt_deviceentcnts_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGEVENTCOUNTERS */
int32_t dw3000_ioc_configeventcounters(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_configeventcounters(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_SETPREAMBLEDETECTTIMEOUT */
int32_t dw3000_ioc_setpreambledetecttimeout(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            ull_setpreambledetecttimeout(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETSNIFFMODE */
int32_t dw3000_ioc_setsniffmode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_set_sniff_mode_s *tmp = (struct dwt_set_sniff_mode_s *)ptr;
            ull_setsniffmode(dw, tmp->enable, tmp->timeOn, tmp->timeOff);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETRXTIMEOUT */
int32_t dw3000_ioc_setrxtimeout(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            ull_setrxtimeout(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_AONREAD */
int32_t dw3000_ioc_aonread(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_aon_read_s *tmp = (struct dwt_aon_read_s *)ptr;
            tmp->ret_val = ull_aon_read(dw, tmp->aon_address);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_AONWRITE */
int32_t dw3000_ioc_aonwrite(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_aon_write_s *tmp = (struct dwt_aon_write_s *)ptr;
            ull_aon_write(dw, tmp->aon_address, tmp->aon_write_data);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READSTSSTATUS */
int32_t dw3000_ioc_readstsstatus(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ret = ull_readstsstatus(dw, (uint16_t *)ptr, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_SETLEDS */
int32_t dw3000_ioc_setleds(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            ull_setleds(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETDWSTATE */
int32_t dw3000_ioc_setdwstate(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ret = ull_setdwstate(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_READSYSTIME */
int32_t dw3000_ioc_readsystime(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_readsystime(dw, (uint8_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_CHECKIDLERC */
int32_t dw3000_ioc_checkidlerc(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_checkidlerc(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CHECKIRQ */
int32_t dw3000_ioc_checkirq(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_checkirq(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGUREFRAMEFILTER */
int32_t dw3000_ioc_configureframefilter(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_configure_ff_s *tmp = (struct dwt_configure_ff_s *)ptr;
            ull_configureframefilter(dw, tmp->enabletype, tmp->filtermode);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETEUI */
int32_t dw3000_ioc_seteui(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_seteui(dw, (uint8_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_GETEUI */
int32_t dw3000_ioc_geteui(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_geteui(dw, (uint8_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_SETPANID */
int32_t dw3000_ioc_setpanid(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            ull_setpanid(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETADDRESS16 */
int32_t dw3000_ioc_setaddress16(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            ull_setaddress16(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READRXTIMESTAMP */
int32_t dw3000_ioc_readrxtimestamp(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_readrxtimestamp(dw, (uint8_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_READRXTIMESTAMP_IPATOV */
int32_t dw3000_ioc_readrxtimestamp_ipatov(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_readrxtimestamp_ipatov(dw, (uint8_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_READRXTIMESTAMPUNADJ */
int32_t dw3000_ioc_readrxtimestampunadj(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_readrxtimestampunadj(dw, (uint8_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_READRXTIMESTAMPHI32 */
int32_t dw3000_ioc_readrxtimestamphi32(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = ull_readrxtimestamphi32(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READRXTIMESTAMPLO32 */
int32_t dw3000_ioc_readrxtimestamplo32(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = ull_readrxtimestamplo32(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READRXTIMESTAMP_STS */
int32_t dw3000_ioc_readrxtimestamp_sts(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_readrxtimestamp_sts(dw, (uint8_t *)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_READSYSTIMESTAMPHI32 */
int32_t dw3000_ioc_readsystimestamphi32(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = ull_readsystimehi32(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_OTPREAD */
int32_t dw3000_ioc_otpread(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_otp_read_s *d = (struct dwt_otp_read_s *)ptr;
            ull_otpread(dw, d->address, d->array, d->length);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETPLENFINE */
int32_t dw3000_ioc_setplenfine(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            ull_setplenfine(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_RUNPGFCAL */
int32_t dw3000_ioc_runpgfcal(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ret = ull_run_pgfcal(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_PGF_CAL */
int32_t dw3000_ioc_pgf_cal(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ret = ull_pgf_cal(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_CALCPGCOUNT */
int32_t dw3000_ioc_calcpgcount(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_calc_pg_count_s *tmp = (struct dwt_calc_pg_count_s *)ptr;
            tmp->result = ull_calcpgcount(dw, tmp->pgdly);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_PLL_STATUS */
int32_t dw3000_ioc_pll_status(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ret = (int32_t)ull_readpllstatus(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_PLL_CAL */
int32_t dw3000_ioc_pll_cal(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ret = ull_pll_cal(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_CONFIGURE_RF_PORT */
int32_t dw3000_ioc_configure_rf_port(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
    {
        dwt_rf_port_ctrl_e port_control = (dwt_rf_port_ctrl_e)parm;
        ull_configure_rf_port(dw, port_control);
    }
    break;
    } while (false);

    return ret;
}

/* DWT_SETGPIOMODE */
int32_t dw3000_ioc_setgpiomode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_set_gpio_mode_s *tmp = (struct dwt_set_gpio_mode_s *)ptr;
            ull_setgpiomode(dw, tmp->mask, tmp->mode);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETGPIODIR */
int32_t dw3000_ioc_setgpiodir(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            ull_setgpiodir(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETGPIOVALUE */
int32_t dw3000_ioc_setgpiovalue(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_set_gpio_value_s *tmp = (struct dwt_set_gpio_value_s *)ptr;
            ull_setgpiovalue(dw, tmp->gpio, tmp->value);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETDBLRXBUFFMODE */
int32_t dw3000_ioc_setdblrxbuffmode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_set_dbl_rx_buff_mode_s *tmp = (struct dwt_set_dbl_rx_buff_mode_s *)ptr;
            ull_setdblrxbuffmode(dw, tmp->dbl_buff_state, tmp->dbl_buff_mode);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETREFERENCETRXTIME */
int32_t dw3000_ioc_setreferencetrxtime(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            ull_setreferencetrxtime(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READ_REG */
int32_t dw3000_ioc_read_reg(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = dwt_read32bitreg(dw, (uint32_t)parm);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_WRITE_REG */
int32_t dw3000_ioc_write_reg(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        dwt_write32bitreg(dw, (uint32_t)parm, (uint32_t)ptr);
        break;
    } while (false);

    return ret;
}

/* DWT_GETDGCDECISION */
int32_t dw3000_ioc_getdgcdecision(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_get_dgcdecision(dw);
        }
        break;
    } while (false);

    return ret;
}

#ifdef WIN32
/* DWT_SPICSWAKEUP */
int32_t dw3000_ioc_spicswakeup(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_spi_cs_wakeup_s *tmp = (struct dwt_spi_cs_wakeup_s *)ptr;
            ret = ull_spicswakeup(dw, tmp->buff, tmp->length);
        }
        break;
    } while (false);

    return ret;
}
#endif /* WIN32 */

/* DWT_WRITESYSSTATUSLO */
int32_t dw3000_ioc_writesysstatuslo(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            ull_writesysstatuslo(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_WRITESYSSTATUSHI */
int32_t dw3000_ioc_writesysstatushi(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            /*
             * DW3000 SYS_STATUS_HI register is 13 bits wide, thus we revert to a 16-bit write of the register
             * instead of using a 32-bit write. Only DW3720 devices require a 32-bit write.
             */
            uint32_t *tmp = (uint32_t *)ptr;
            ull_writesysstatushi(dw, (uint16_t)*tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READSYSSTATUSLO */
int32_t dw3000_ioc_readsysstatuslo(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = ull_readsysstatuslo(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READSYSSTATUSHI */
int32_t dw3000_ioc_readsysstatushi(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            /*
             * DW3000 SYS_STATUS_HI register is 13 bits wide, thus we revert to a 16-bit read of the register
             * instead of using a 32-bit read. Only DW3720 devices require a 32-bit write.
             */
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = (uint32_t)ull_readsysstatushi(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_WRITERDBSTATUS */
int32_t dw3000_ioc_writerdbstatus(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            ull_writerdbstatus(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READRDBSTATUS */
int32_t dw3000_ioc_readrdbstatus(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            *tmp = ull_readrdbstatus(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_GETFRAMELENGTH */
int32_t dw3000_ioc_getframelength(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            uint8_t rng_bit;
            *tmp = ull_getframelength(dw, &rng_bit);
            (void)rng_bit; // Ranging bit is currently ignored
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READGPIOVALUE */
int32_t dw3000_ioc_readgpiovalue(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *tmp = (uint16_t *)ptr;
            *tmp = ull_readgpiovalue(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READPDOAOFFSET */
int32_t dw3000_ioc_readpdoaoffset(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = dwt_read32bitreg(dw, CIA_ADJUST_ID);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETPDOAOFFSET */
int32_t dw3000_ioc_setpdoaoffset(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t tmp = *((uint16_t *)ptr);
            tmp &= CIA_ADJUST_PDOA_ADJ_OFFSET_BIT_MASK;
            dwt_modify16bitoffsetreg(dw, CIA_ADJUST_ID, 0U, (uint16_t)~CIA_ADJUST_PDOA_ADJ_OFFSET_BIT_MASK, tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_ADJ_TXPOWER */
int32_t dw3000_ioc_adj_txpower(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_adj_tx_power_s *tmp = (struct dwt_adj_tx_power_s *)ptr;
            tmp->result = ull_adjust_tx_power(tmp->boost, tmp->ref_tx_power, tmp->channel, tmp->adj_tx_power, tmp->applied_boost);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_LINEAR_TXPOWER */
int32_t dw3000_ioc_linear_txpower(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_calculate_linear_tx_setting_s *tmp = (struct dwt_calculate_linear_tx_setting_s *)ptr;
            tmp->result = ull_calculate_linear_tx_setting(dw, tmp->channel, tmp->txp_indexes, tmp->txp_res);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CONVERT_TXPOWER_TO_IDX */
int32_t dw3000_ioc_convert_txpower_to_idx(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_convert_tx_power_to_index_s *tmp = (struct dwt_convert_tx_power_to_index_s *)ptr;
            tmp->result = ull_convert_tx_power_to_index(tmp->channel, tmp->tx_power, tmp->tx_power_idx);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SET_PLL_CONFIG */
int32_t dw3000_ioc_set_pll_config(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t tmp = *((uint32_t *)ptr);
            ull_set_pll_config(dw, tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CFGWIFICOEXSET */
int32_t dw3000_ioc_cfgwificoexset(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_cfg_wifi_coex_set_s *tmp = (struct dwt_cfg_wifi_coex_set_s *)ptr;
            ull_wifi_coex_set(dw, tmp->enable, tmp->coex_io_swap);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CFGANTSEL */
int32_t dw3000_ioc_cfgantsel(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint8_t *tmp = (uint8_t *)ptr;
            ull_configure_and_set_antenna_selection_gpio(dw, *tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_RSTSYSTEMCNT */
int32_t dw3000_ioc_rstsystemcnt(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_reset_system_counter(dw);
        break;
    } while (false);

    return ret;
}

/* DWT_CFGOSTRMODE */
int32_t dw3000_ioc_cfgostrmode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_ostr_mode_s *tmp = (struct dwt_ostr_mode_s *)ptr;
            ull_config_ostr_mode(dw, tmp->enable, tmp->wait_time);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_PLL_AUTO_CAL */
int32_t dw3000_ioc_pll_auto_cal(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_set_pll_cal_s *tmp = (struct dwt_set_pll_cal_s *)ptr;
            uint8_t steps_to_lock;

            if (parm == 5)
            {
                ret = (int32_t)ull_pll_ch5_auto_cal(dw, tmp->coarse_code, tmp->sleep,
                                                    tmp->steps, &steps_to_lock, tmp->temp);
            }
            else
            {
                ret = (int32_t)ull_pll_ch9_auto_cal(dw, tmp->coarse_code, tmp->sleep,
                                                    tmp->steps, &steps_to_lock);
            }
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SET_STS_LEN */
int32_t dw3000_ioc_set_sts_len(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            dwt_sts_lengths_e *sts_len = (dwt_sts_lengths_e *)ptr;
            ull_setstslength(dw, *sts_len);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CFG_STS */
int32_t dw3000_ioc_cfg_sts(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            dwt_config_t *config = (dwt_config_t *)ptr;

            if (config->stsMode != DWT_STS_MODE_OFF)
            {
                uint16_t sts_mnth;
                uint32_t sts_config_lo_u32;
                // configure CIA STS lower bound
                if ((config->pdoaMode == DWT_PDOA_M1) || (config->pdoaMode == DWT_PDOA_M0))
                {
                    // In PDOA mode 1, number of accumulated symbols is the whole length of the STS
                    sts_mnth = get_sts_mnth(sts_length_factors[(uint8_t)(config->stsLength)], CIA_MANUALLOWERBOUND_TH_64, 3U);
                }
                else
                {
                    // In PDOA mode 3 number of accumulated symbols is half of the length of STS symbols
                    sts_mnth = get_sts_mnth(sts_length_factors[(uint8_t)(config->stsLength)], CIA_MANUALLOWERBOUND_TH_64, 4U);
                }

                sts_config_lo_u32 = ~(STS_CONFIG_LO_STS_MAN_TH_BIT_MASK >> 16U);
                dwt_modify16bitoffsetreg(dw, STS_CONFIG_LO_ID, 2U, (uint16_t)sts_config_lo_u32, sts_mnth & 0x7FU);
                dwt_write8bitoffsetreg(dw, STS_CONFIG_HI_ID, 0U, (uint8_t)STS_CONFIG_HI_RES);
            }
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SETPDOAMODE */
int32_t dw3000_ioc_setpdoamode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ret = ull_setpdoamode(dw, (dwt_pdoa_mode_e)parm);
        break;
    } while (false);

    return ret;
}

/* DWT_SET_FCS_MODE */
int32_t dw3000_ioc_set_fcs_mode(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            dwt_fcs_mode_e *fcs_mode = (dwt_fcs_mode_e *)ptr;

            uint32_t fcs = dwt_read32bitoffsetreg(dw, SYS_CFG_ID, 0U) & ~(SYS_CFG_DIS_FCS_TX_BIT_MASK | SYS_CFG_DIS_FCE_BIT_MASK);

            if (((uint8_t)*fcs_mode & (uint8_t)DWT_FCS_TX_OFF) != 0U)
            {
                fcs |= SYS_CFG_DIS_FCS_TX_BIT_MASK;
            }

            if (((uint8_t)*fcs_mode & (uint8_t)DWT_FCS_RX_OFF) != 0U)
            {
                fcs |= SYS_CFG_DIS_FCE_BIT_MASK;
            }

            dwt_write32bitoffsetreg(dw, SYS_CFG_ID, 0U, fcs);

            LOCAL_DATA(dw)->sys_cfg_dis_fce_bit_flag = ((fcs & SYS_CFG_DIS_FCE_BIT_MASK) != 0UL ? 1U : 0U);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SET_PHR */
int32_t dw3000_ioc_set_phr(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_set_phr_s *tmp = (struct dwt_set_phr_s *)ptr;
            dwt_modify32bitoffsetreg(dw, SYS_CFG_ID, 0U, ~(SYS_CFG_PHR_MODE_BIT_MASK | SYS_CFG_PHR_6M8_BIT_MASK),
                                     ((SYS_CFG_PHR_6M8_BIT_MASK & ((uint32_t)tmp->phrRate << SYS_CFG_PHR_6M8_BIT_OFFSET)) | (uint32_t)tmp->phrMode));
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SET_DATARATE */
int32_t dw3000_ioc_set_datarate(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            dwt_uwb_bit_rate_e *bitRate = (dwt_uwb_bit_rate_e *)ptr;
            dwt_modify32bitoffsetreg(dw, TX_FCTRL_ID, 0U, ~(TX_FCTRL_TXBR_BIT_MASK), (uint32_t)*bitRate << TX_FCTRL_TXBR_BIT_OFFSET);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SET_PAC */
int32_t dw3000_ioc_set_pac(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            dwt_pac_size_e *rxPAC = (dwt_pac_size_e *)ptr;
            dwt_modify8bitoffsetreg(dw, DTUNE0_ID, 0U, (uint8_t)~(DTUNE0_PRE_PAC_SYM_BIT_MASK), (uint8_t)*rxPAC);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SET_SFDTO */
int32_t dw3000_ioc_set_sfdto(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint16_t *sfdTO = (uint16_t *)ptr;
            if (*sfdTO == 0U)
            {
                *sfdTO = DWT_SFDTOC_DEF;
            }
            dwt_write16bitoffsetreg(dw, DTUNE0_ID, 2U, *sfdTO);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READDGCDBG */
int32_t dw3000_ioc_readdgcdbg(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = dwt_read32bitreg(dw, DGC_DBG_ID);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_READCTRDBG */
int32_t dw3000_ioc_readctrdbg(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = dwt_read32bitreg(dw, CTR_DBG_ID);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_GET_CIR_REGADD */
int32_t dw3000_ioc_get_cir_regadd(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = ACC_MEM_ID;
        }
        break;
    } while (false);

    return ret;
}

/* DWT_DBG_REGS */
int32_t dw3000_ioc_dbg_regs(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        // if (ptr) - ptr will be NULL
        {
            ptr = (void *)regNames;
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CIA_VERSION */
int32_t dw3000_ioc_cia_version(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            uint32_t *tmp = (uint32_t *)ptr;
            *tmp = ull_readCIAversion(dw);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_NLOS_IPDIAG */
int32_t dw3000_ioc_nlos_ipdiag(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            dwt_nlos_ipdiag_t *tmp = (dwt_nlos_ipdiag_t *)ptr;
            ull_nlos_ipdiag(dw, tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_NLOS_ALLDIAG */
int32_t dw3000_ioc_nlos_alldiag(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            dwt_nlos_alldiag_t *tmp = (dwt_nlos_alldiag_t *)ptr;
            tmp->result = ull_nlos_alldiag(dw, tmp);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_DIS_OTP_IPS */
int32_t dw3000_ioc_dis_otp_ips(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        ull_dis_otp_ips(dw, parm);
        break;
    } while (false);

    return ret;
}

/* DWT_CALCULATE_RSSI */
int32_t dw3000_ioc_calculate_rssi(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_calculate_rssi_s *tmp = (struct dwt_calculate_rssi_s *)ptr;
            ret = ull_calculate_rssi(dw, tmp->cir_diagnostics, tmp->acc_idx, tmp->signal_strength);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_CALCULATE_FIRST_PATH_POWER */
int32_t dw3000_ioc_calculate_first_path_power(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        if (ptr != NULL)
        {
            struct dwt_calculate_first_path_power_s *tmp = (struct dwt_calculate_first_path_power_s *)ptr;
            ret = ull_calculate_first_path_power(dw, tmp->cir_diagnostics, tmp->acc_idx, tmp->signal_strength);
        }
        break;
    } while (false);

    return ret;
}

/* DWT_SET_ISR_FLAGS */
int32_t dw3000_ioc_set_isr_flags(dwchip_t *dw, int32_t parm, void *ptr)
{
    int32_t ret = (int32_t)DWT_SUCCESS;
    (void)dw;
    (void)parm;
    (void)ptr;

    do
    {
        dw->isrFlags = (dwt_isr_flags_e)parm;
        break;
    } while (false);

    return ret;
}

