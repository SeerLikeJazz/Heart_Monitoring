/**
 * Copyright (c) 2013 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_LBS)
#include "ble_lbs.h"
#include "ble_srv_common.h"


/**д����
 *
 * @param[in] p_lbs      LED Button Service structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
static void on_write(ble_lbs_t * p_lbs, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (   (p_evt_write->handle == p_lbs->led_char_handles.value_handle)
        && (p_evt_write->len == 1)
        && (p_lbs->led_write_handler != NULL))
    {
        p_lbs->led_write_handler(p_ble_evt->evt.gap_evt.conn_handle, p_lbs, p_evt_write->data[0]);
    }
}

//
void ble_lbs_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_lbs_t * p_lbs = (ble_lbs_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTS_EVT_WRITE:
            on_write(p_lbs, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

//�����ʼ��
uint32_t ble_lbs_init(ble_lbs_t * p_lbs, const ble_lbs_init_t * p_lbs_init)
{
    uint32_t              err_code;
    ble_uuid_t            ble_uuid;
    ble_add_char_params_t add_char_params;

    // ��ʼ��������----������Ľ���
    p_lbs->led_write_handler = p_lbs_init->led_write_handler;

    // Add service.
    ble_uuid128_t base_uuid = {LBS_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_lbs->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_lbs->uuid_type;
    ble_uuid.uuid = LBS_UUID_SERVICE;
   
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_lbs->service_handle);
    VERIFY_SUCCESS(err_code);
     
// ���밴������
    memset(&add_char_params, 0, sizeof(add_char_params));
		//GATT����
    add_char_params.uuid              = LBS_UUID_BUTTON_CHAR;//��Ӱ�����UUID
    add_char_params.uuid_type         = p_lbs->uuid_type;//UUID����
    add_char_params.init_len          = sizeof(uint8_t);//��ʼ�����ݳ���
    add_char_params.max_len           = sizeof(uint8_t);//������ݳ���
		//��������
    add_char_params.char_props.read   = 1;
    add_char_params.char_props.notify = 1;
   //GATT����
    add_char_params.read_access       = SEC_OPEN;//��ȫ����
    add_char_params.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_lbs->service_handle,//���η���ľ��
                                  &add_char_params,
                                  &p_lbs->button_char_handles);//�������Եľ��
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
		
   // ����DHT����
    memset(&add_char_params, 0, sizeof(add_char_params));
		//GATT����
    add_char_params.uuid              = LBS_UUID_DHT11_CHAR;//��Ӱ�����UUID
    add_char_params.uuid_type         = p_lbs->uuid_type;//UUID����
    add_char_params.init_len          = 15;//��ʼ�����ݳ���
    add_char_params.max_len           = 15;//������ݳ���
		//��������
    add_char_params.char_props.read   = 1;
    add_char_params.char_props.notify = 1;
   //GATT����
    add_char_params.read_access       = SEC_OPEN;//��ȫ����
    add_char_params.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_lbs->service_handle,//���η���ľ��
                                  &add_char_params,
                                  &p_lbs->DHT11_char_handles);//�������Եľ��
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }



		// ���LED������
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid             = LBS_UUID_LED_CHAR;//���Ե�UUID
    add_char_params.uuid_type        = p_lbs->uuid_type;//UUID����
    add_char_params.init_len         = sizeof(uint8_t);//1���ֽ�
    add_char_params.max_len          = sizeof(uint8_t);//��󳤶�1���ֽ�
    add_char_params.char_props.read  = 1;//������
    add_char_params.char_props.write = 1;//д����

    add_char_params.read_access  = SEC_OPEN;//
    add_char_params.write_access = SEC_OPEN;

    return characteristic_add(p_lbs->service_handle, &add_char_params, &p_lbs->led_char_handles);
		
		
		
}
//��Ӱ���֪ͨ
uint32_t ble_lbs_on_button_change(uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t button_state)
{
    ble_gatts_hvx_params_t params;
    uint16_t len = sizeof(button_state);

    memset(&params, 0, sizeof(params));
    params.type   = BLE_GATT_HVX_NOTIFICATION;//֪ͨ
    params.handle = p_lbs->button_char_handles.value_handle;//�������
    params.p_data = &button_state;//�ϴ�������
    params.p_len  = &len;//����

    return sd_ble_gatts_hvx(conn_handle, &params);//�ϴ�����
}

//�����ʪ���ϴ�����
uint16_t ble_lbs_on_dht_change(uint16_t conn_handle,ble_lbs_t * p_lbs, uint8_t dht11tempval,uint8_t dht11humival)
{
    ble_gatts_hvx_params_t params;
    uint16_t len = 2;
	  uint32_t err_code;
	  uint8_t  thi_val[2];
	
	
	  thi_val[0] = (uint8_t)dht11tempval;  //�¶���������
	  thi_val[1] = (uint8_t)dht11humival;    //�¶�С������
	 
	
	  if (conn_handle != BLE_CONN_HANDLE_INVALID)
		{
			memset(&params, 0, sizeof(params));
      params.type = BLE_GATT_HVX_NOTIFICATION;
      params.handle = p_lbs->DHT11_char_handles.value_handle;
      params.p_data = thi_val;
      params.p_len = &len;
    
      return sd_ble_gatts_hvx(conn_handle, &params);
		}
		else{
			err_code = NRF_ERROR_INVALID_STATE;
		}
    return err_code;
    
}

//����Ե������ϴ�����
uint16_t ble_lbs_on_ads1292_change(uint16_t conn_handle,ble_lbs_t * p_lbs, uint8_t *ads)
{
    ble_gatts_hvx_params_t params;
    uint16_t len = 15;
	  uint32_t err_code;
	
	  if (conn_handle != BLE_CONN_HANDLE_INVALID)
		{
			memset(&params, 0, sizeof(params));
      params.type = BLE_GATT_HVX_NOTIFICATION;
      params.handle = p_lbs->DHT11_char_handles.value_handle;
      params.p_data = ads;
      params.p_len = &len;
    
      return sd_ble_gatts_hvx(conn_handle, &params);
		}
		else{
			err_code = NRF_ERROR_INVALID_STATE;
		}
    return err_code;
    
}

#endif // NRF_MODULE_ENABLED(BLE_LBS)
