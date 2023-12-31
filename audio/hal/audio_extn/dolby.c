/*
 * Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "audio_hw_dolby"
/*#define LOG_NDEBUG 0*/
#define LOG_NDDEBUG 0

#include <errno.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <cutils/str_parms.h>
#include <log/log.h>

#include "audio_hw.h"
#include "platform.h"
#include "platform_api.h"
#include "audio_extn.h"
#include "sound/compress_params.h"

#ifdef DS1_DOLBY_DDP_ENABLED

#define AUDIO_PARAMETER_DDP_DEV          "ddp_device"
#define AUDIO_PARAMETER_DDP_CH_CAP       "ddp_chancap"
#define AUDIO_PARAMETER_DDP_MAX_OUT_CHAN "ddp_maxoutchan"
#define AUDIO_PARAMETER_DDP_OUT_MODE     "ddp_outmode"
#define AUDIO_PARAMETER_DDP_OUT_LFE_ON   "ddp_outlfeon"
#define AUDIO_PARAMETER_DDP_COMP_MODE    "ddp_compmode"
#define AUDIO_PARAMETER_DDP_STEREO_MODE  "ddp_stereomode"

#define PARAM_ID_MAX_OUTPUT_CHANNELS    0x00010DE2
#define PARAM_ID_CTL_RUNNING_MODE       0x0
#define PARAM_ID_CTL_ERROR_CONCEAL      0x00010DE3
#define PARAM_ID_CTL_ERROR_MAX_RPTS     0x00010DE4
#define PARAM_ID_CNV_ERROR_CONCEAL      0x00010DE5
#define PARAM_ID_CTL_SUBSTREAM_SELECT   0x00010DE6
#define PARAM_ID_CTL_INPUT_MODE         0x0
#define PARAM_ID_OUT_CTL_OUTMODE        0x00010DE0
#define PARAM_ID_OUT_CTL_OUTLFE_ON      0x00010DE1
#define PARAM_ID_OUT_CTL_COMPMODE       0x00010D74
#define PARAM_ID_OUT_CTL_STEREO_MODE    0x00010D76
#define PARAM_ID_OUT_CTL_DUAL_MODE      0x00010D75
#define PARAM_ID_OUT_CTL_DRCSCALE_HIGH  0x00010D7A
#define PARAM_ID_OUT_CTL_DRCSCALE_LOW   0x00010D79
#define PARAM_ID_OUT_CTL_OUT_PCMSCALE   0x00010D78
#define PARAM_ID_OUT_CTL_MDCT_BANDLIMIT 0x00010DE7
#define PARAM_ID_OUT_CTL_DRC_SUPPRESS   0x00010DE8

/* DS1-DDP Endp Params */
#define DDP_ENDP_NUM_PARAMS 17
#define DDP_ENDP_NUM_DEVICES 21
static int ddp_endp_params_id[DDP_ENDP_NUM_PARAMS] = {
    PARAM_ID_MAX_OUTPUT_CHANNELS, PARAM_ID_CTL_RUNNING_MODE,
    PARAM_ID_CTL_ERROR_CONCEAL, PARAM_ID_CTL_ERROR_MAX_RPTS,
    PARAM_ID_CNV_ERROR_CONCEAL, PARAM_ID_CTL_SUBSTREAM_SELECT,
    PARAM_ID_CTL_INPUT_MODE, PARAM_ID_OUT_CTL_OUTMODE,
    PARAM_ID_OUT_CTL_OUTLFE_ON, PARAM_ID_OUT_CTL_COMPMODE,
    PARAM_ID_OUT_CTL_STEREO_MODE, PARAM_ID_OUT_CTL_DUAL_MODE,
    PARAM_ID_OUT_CTL_DRCSCALE_HIGH, PARAM_ID_OUT_CTL_DRCSCALE_LOW,
    PARAM_ID_OUT_CTL_OUT_PCMSCALE, PARAM_ID_OUT_CTL_MDCT_BANDLIMIT,
    PARAM_ID_OUT_CTL_DRC_SUPPRESS
};

static struct ddp_endp_params {
    int  device;
    int  dev_ch_cap;
    int  param_val[DDP_ENDP_NUM_PARAMS];
    bool is_param_valid[DDP_ENDP_NUM_PARAMS];
} ddp_endp_params[DDP_ENDP_NUM_DEVICES] = {
          {AUDIO_DEVICE_OUT_EARPIECE, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 } },
          {AUDIO_DEVICE_OUT_SPEAKER, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_WIRED_HEADSET, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_WIRED_HEADPHONE, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_BLUETOOTH_SCO, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_AUX_DIGITAL, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 2, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_AUX_DIGITAL, 6,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 2, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_AUX_DIGITAL, 8,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 2, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_USB_ACCESSORY, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_USB_DEVICE, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_FM, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_FM_TX, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 6, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_PROXY, 2,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 2, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
          {AUDIO_DEVICE_OUT_PROXY, 6,
              {8, 0, 0, 0, 0, 0, 0, 21, 1, 2, 0, 0, 0, 0, 0, 0, 0},
              {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0} },
};

int update_ddp_endp_table(int device, int dev_ch_cap, int param_id,
                          int param_val)
{
    int idx = 0;
    int param_idx = 0;
    ALOGV("%s: dev 0x%x dev_ch_cap %d param_id 0x%x param_val %d",
           __func__, device, dev_ch_cap , param_id, param_val);

    for(idx=0; idx<DDP_ENDP_NUM_DEVICES; idx++) {
        if(ddp_endp_params[idx].device == device) {
            if(ddp_endp_params[idx].dev_ch_cap == dev_ch_cap) {
                break;
            }
        }
    }

    if(idx>=DDP_ENDP_NUM_DEVICES) {
        ALOGE("%s: device not available in DDP endp config table", __func__);
        return -EINVAL;
    }

    for(param_idx=0; param_idx<DDP_ENDP_NUM_PARAMS; param_idx++) {
        if (ddp_endp_params_id[param_idx] == param_id) {
            break;
        }
    }

    if(param_idx>=DDP_ENDP_NUM_PARAMS) {
        ALOGE("param not available in DDP endp config table");
        return -EINVAL;
    }

    ALOGV("ddp_endp_params[%d].param_val[%d] = %d", idx, param_idx, param_val);
    ddp_endp_params[idx].param_val[param_idx] = param_val;
    return 0;
}

void send_ddp_endp_params_stream(struct stream_out *out,
                                 int device, int dev_ch_cap,
                                 bool set_cache __unused)
{
    int idx, i;
    int ddp_endp_params_data[2*DDP_ENDP_NUM_PARAMS + 1];
    int length = 0;
    for(idx=0; idx<DDP_ENDP_NUM_DEVICES; idx++) {
        if(ddp_endp_params[idx].device & device) {
            if(ddp_endp_params[idx].dev_ch_cap == dev_ch_cap) {
                break;
            }
        }
    }
    if(idx>=DDP_ENDP_NUM_DEVICES) {
        ALOGE("device not available in DDP endp config table");
        return;
    }

    length += 1; /* offset 0 is for num of parameter. increase offset by 1 */
    for (i=0; i<DDP_ENDP_NUM_PARAMS; i++) {
        if(ddp_endp_params[idx].is_param_valid[i]) {
            ddp_endp_params_data[length++] = ddp_endp_params_id[i];
            ddp_endp_params_data[length++] = ddp_endp_params[idx].param_val[i];
        }
    }
    ddp_endp_params_data[0] = (length-1)/2;
    if(length) {
        char mixer_ctl_name[128];
        struct audio_device *adev = out->dev;
        struct mixer_ctl *ctl;
        int pcm_device_id = platform_get_pcm_device_id(out->usecase,
                                                       PCM_PLAYBACK);
        snprintf(mixer_ctl_name, sizeof(mixer_ctl_name),
                 "Audio Stream %d Dec Params", pcm_device_id);
        ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
        if (!ctl) {
            ALOGE("%s: Could not get ctl for mixer cmd - %s",
                  __func__, mixer_ctl_name);
            return;
        }
        mixer_ctl_set_array(ctl, ddp_endp_params_data, length);
    }
    return;
}

void send_ddp_endp_params(struct audio_device *adev,
                          int ddp_dev, int dev_ch_cap)
{
    struct listnode *node;
    struct audio_usecase *usecase;

    list_for_each(node, &adev->usecase_list) {
        usecase = node_to_item(node, struct audio_usecase, list);
        if ((usecase->type == PCM_PLAYBACK) &&
            (usecase->devices & ddp_dev) &&
            (usecase->stream.out->flags & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) &&
            ((usecase->stream.out->format == AUDIO_FORMAT_AC3) ||
             (usecase->stream.out->format == AUDIO_FORMAT_E_AC3))) {
            send_ddp_endp_params_stream(usecase->stream.out, ddp_dev,
                                        dev_ch_cap, false /* set cache */);
        }
    }
}

void audio_extn_dolby_send_ddp_endp_params(struct audio_device *adev)
{
    struct listnode *node;
    struct audio_usecase *usecase;
    list_for_each(node, &adev->usecase_list) {
        usecase = node_to_item(node, struct audio_usecase, list);
        if ((usecase->type == PCM_PLAYBACK) &&
            (usecase->devices & AUDIO_DEVICE_OUT_ALL) &&
            (usecase->stream.out->flags & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) &&
            ((usecase->stream.out->format == AUDIO_FORMAT_AC3) ||
             (usecase->stream.out->format == AUDIO_FORMAT_E_AC3))) {
            /*
             * Use wfd /hdmi sink channel cap for dolby params if device is wfd
             * or hdmi. Otherwise use stereo configuration
             */
            int channel_cap = usecase->devices & AUDIO_DEVICE_OUT_AUX_DIGITAL ?
                              adev->cur_hdmi_channels :
                              usecase->devices & AUDIO_DEVICE_OUT_PROXY ?
                              adev->cur_wfd_channels : 2;
            send_ddp_endp_params_stream(usecase->stream.out, usecase->devices,
                                        channel_cap, false /* set cache */);
        }
    }
}

void audio_extn_ddp_set_parameters(struct audio_device *adev,
                                   struct str_parms *parms)
{
    int ddp_dev, dev_ch_cap;
    int val, ret;
    char value[32]={0};
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_DDP_DEV, value,
                            sizeof(value));
    if (ret >= 0) {
        ddp_dev = atoi(value);
        if (!(AUDIO_DEVICE_OUT_ALL & ddp_dev))
            return;
    } else
        return;

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_DDP_CH_CAP, value,
                            sizeof(value));
    if (ret >= 0) {
        dev_ch_cap = atoi(value);
        if ((dev_ch_cap != 2) && (dev_ch_cap != 6) && (dev_ch_cap != 8))
            return;
    } else
        return;

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_DDP_MAX_OUT_CHAN, value,
                            sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        update_ddp_endp_table(ddp_dev, dev_ch_cap,
                              PARAM_ID_MAX_OUTPUT_CHANNELS, val);
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_DDP_OUT_MODE, value,
                            sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        update_ddp_endp_table(ddp_dev, dev_ch_cap,
                              PARAM_ID_OUT_CTL_OUTMODE, val);
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_DDP_OUT_LFE_ON, value,
                            sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        update_ddp_endp_table(ddp_dev, dev_ch_cap,
                              PARAM_ID_OUT_CTL_OUTLFE_ON, val);
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_DDP_COMP_MODE, value,
                            sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        update_ddp_endp_table(ddp_dev, dev_ch_cap,
                              PARAM_ID_OUT_CTL_COMPMODE, val);
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_DDP_STEREO_MODE, value,
                            sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        update_ddp_endp_table(ddp_dev, dev_ch_cap,
                              PARAM_ID_OUT_CTL_STEREO_MODE, val);
    }
    /* TODO: Do we need device channel caps here?
     * We dont have that information as this is from dolby modules
     */
    send_ddp_endp_params(adev, ddp_dev, dev_ch_cap);
}

int audio_extn_dolby_get_snd_codec_id(struct audio_device *adev,
                                      struct stream_out *out,
                                      audio_format_t format)
{
    int id = 0;
    /*
     * Use wfd /hdmi sink channel cap for dolby params if device is wfd
     * or hdmi. Otherwise use stereo configuration
     */
    int channel_cap = out->devices & AUDIO_DEVICE_OUT_AUX_DIGITAL ?
                      adev->cur_hdmi_channels :
                      out->devices & AUDIO_DEVICE_OUT_PROXY ?
                      adev->cur_wfd_channels : 2;

    switch (format) {
    case AUDIO_FORMAT_AC3:
        id = SND_AUDIOCODEC_AC3;
        send_ddp_endp_params_stream(out, out->devices,
                            channel_cap, true /* set_cache */);
#ifndef DS1_DOLBY_DAP_ENABLED
        audio_extn_dolby_set_dmid(adev);
#endif
        break;
    case AUDIO_FORMAT_E_AC3:
        id = SND_AUDIOCODEC_EAC3;
        send_ddp_endp_params_stream(out, out->devices,
                            channel_cap, true /* set_cache */);
#ifndef DS1_DOLBY_DAP_ENABLED
        audio_extn_dolby_set_dmid(adev);
#endif
        break;
    default:
        ALOGE("%s: Unsupported audio format :%x", __func__, format);
    }

    return id;
}

bool audio_extn_is_dolby_format(audio_format_t format)
{
    if (format == AUDIO_FORMAT_AC3 ||
            format == AUDIO_FORMAT_E_AC3)
        return true;
    else
        return false;
}

#endif /* DS1_DOLBY_DDP_ENABLED */

#ifdef DS1_DOLBY_DAP_ENABLED
void audio_extn_dolby_set_endpoint(struct audio_device *adev)
{
    struct listnode *node;
    struct audio_usecase *usecase;
    struct mixer_ctl *ctl;
    const char *mixer_ctl_name = "DS1 DAP Endpoint";
    int endpoint = 0, ret;
    bool send = false;

    list_for_each(node, &adev->usecase_list) {
        usecase = node_to_item(node, struct audio_usecase, list);
        if ((usecase->type == PCM_PLAYBACK) &&
            (usecase->id != USECASE_AUDIO_PLAYBACK_LOW_LATENCY)) {
            endpoint |= usecase->devices & AUDIO_DEVICE_OUT_ALL;
            send = true;
        }
    }
    if (!send)
        return;

    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        return;
    }
    ret = mixer_ctl_set_value(ctl, 0, endpoint);
    if (ret)
        ALOGE("%s: Dolby set endpint cannot be set error:%d",__func__, ret);

    return;
}
#endif /* DS1_DOLBY_DAP_ENABLED */


#if defined(DS1_DOLBY_DDP_ENABLED) || defined(DS1_DOLBY_DAP_ENABLED)
void audio_extn_dolby_set_dmid(struct audio_device *adev)
{
    struct listnode *node;
    struct audio_usecase *usecase;
    struct mixer_ctl *ctl;
    const char *mixer_ctl_name = "DS1 Security";
    char c_dmid[128] = {0};
    int i_dmid, ret;
    bool send = false;

    list_for_each(node, &adev->usecase_list) {
        usecase = node_to_item(node, struct audio_usecase, list);
        if (usecase->type == PCM_PLAYBACK)
            send = true;
    }
    if (!send)
        return;

    property_get("vendor.audio.dmid",c_dmid,"0");
    i_dmid = atoll(c_dmid);

    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        return;
    }
    ALOGV("%s Dolby device manufacturer id is:%d",__func__,i_dmid);
    ret = mixer_ctl_set_value(ctl, 0, i_dmid);
    if (ret)
        ALOGE("%s: Dolby DMID cannot be set error:%d",__func__, ret);

    return;
}
#endif /* DS1_DOLBY_DDP_ENABLED || DS1_DOLBY_DAP_ENABLED */
