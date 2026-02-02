/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define _RTW_LED_C_
#include <drv_types.h>

#ifdef CONFIG_RTW_SW_LED
void rtw_led_control(_adapter *adapter, LED_CTL_MODE mode)
{
	struct led_priv *ledpriv = adapter_to_led(adapter);

#if defined(CONFIG_CONCURRENT_MODE)
	if (!is_primary_adapter(adapter))
		return;
#endif

	if (mode == ledpriv->mode &&
		mode != LED_CTL_MANUAL)
		return;

	switch (ledpriv->mode) {
	case LED_CTL_UP_IDLE:
		if (mode == LED_CTL_DOWN)
			_rtw_led_control(adapter, RTW_LED_EVENT_SW_RF_OFF);
		else if (mode == LED_CTL_BUSY)
			_rtw_led_control(adapter, RTW_LED_EVENT_LINKED);
		else if (mode == LED_CTL_MANUAL)
			_rtw_led_control(adapter, RTW_LED_EVENT_DONT_CARE);
#ifdef CONFIG_WIFI_LED_SHARE_WITH_WPS
		else if (mode == LED_CTL_START_WPS) {
			_rtw_led_control(adapter, RTW_LED_EVENT_WPS);
			ledpriv->last_mode = ledpriv->mode;
		}
#endif		
		else
			return;
		break;		
	case LED_CTL_DOWN:		
		if (mode == LED_CTL_UP_IDLE)
			_rtw_led_control(adapter, RTW_LED_EVENT_SW_RF_ON);
		else if (mode == LED_CTL_MANUAL)
			_rtw_led_control(adapter, RTW_LED_EVENT_DONT_CARE);
		else
			return;
		break;
	case LED_CTL_BUSY:		
		if (mode == LED_CTL_UP_IDLE)
			_rtw_led_control(adapter, RTW_LED_EVENT_SW_RF_ON);
		else if (mode == LED_CTL_DOWN)
			_rtw_led_control(adapter, RTW_LED_EVENT_SW_RF_OFF);
		else if (mode == LED_CTL_MANUAL)
			_rtw_led_control(adapter, RTW_LED_EVENT_DONT_CARE);
#ifdef CONFIG_WIFI_LED_SHARE_WITH_WPS
		else if (mode == LED_CTL_START_WPS) {
			ledpriv->last_mode = ledpriv->mode;
		}
#endif
		else
			return;
		break;
	case LED_CTL_MANUAL:
		if (mode == LED_CTL_MANUAL)
			_rtw_led_control(adapter, RTW_LED_EVENT_DONT_CARE);
		else if (mode == LED_CTL_UP_IDLE) {
			if (ledpriv->manual_ctrl == _TRUE)
				return;
			_rtw_led_control(adapter, RTW_LED_EVENT_SW_RF_ON);
		} else if (mode == LED_CTL_DOWN) {
			if (ledpriv->manual_ctrl == _TRUE)
				return;
			_rtw_led_control(adapter, RTW_LED_EVENT_SW_RF_OFF);
		} else
			return;
		break;
#ifdef CONFIG_WIFI_LED_SHARE_WITH_WPS
	case LED_CTL_START_WPS:
		if (mode == LED_CTL_MANUAL)
			_rtw_led_control(adapter, RTW_LED_EVENT_DONT_CARE);
		else if (mode == LED_CTL_DOWN) {
			ledpriv->last_mode = LED_CTL_DOWN;
		} else if (mode == LED_CTL_STOP_WPS) {
			mode = ledpriv->last_mode;
			if (mode == LED_CTL_UP_IDLE)
				_rtw_led_control(adapter, RTW_LED_EVENT_SW_RF_ON);
			else if (mode == LED_CTL_BUSY)
				_rtw_led_control(adapter, RTW_LED_EVENT_LINKED);
		} else {
			ledpriv->last_mode = mode;
			return;
		}
		break;
#endif
	default:
		return;
	}

	ledpriv->mode = mode;
}

void rtw_led_traffic_update(_adapter *adapter)
{
	struct led_priv *ledpriv = adapter_to_led(adapter);
	struct dvobj_priv *pdvobj = adapter_to_dvobj(adapter);
	u64 delta = pdvobj->traffic_stat.cur_tx_bytes + pdvobj->traffic_stat.cur_rx_bytes;

	if (delta > 0)
		rtw_led_control(adapter, LED_CTL_BUSY);
	else
		rtw_led_control(adapter, LED_CTL_UP_IDLE);
}

static void _rtw_led_module_init(_adapter *padapter)
{
	struct led_priv *ledpriv = adapter_to_led(padapter);
	struct rtw_led_action_args_t action_sw_rf_on = {0};
	struct rtw_led_action_args_t action_sw_rf_off = {0};
	struct rtw_led_action_args_t action_linked = {0};
	u32 interval_arr[] = {0};
	
	/* config gpio mode */
	rtw_led_set_ctrl_mode(padapter, RTW_LED_CTRL_SW_PP_MODE);

	/* register event */
	action_sw_rf_on.led_id = ledpriv->led_id;
	action_sw_rf_on.led_action = RTW_LED_ACTION_LOW;
	rtw_led_register_event(padapter, RTW_LED_EVENT_SW_RF_ON, RTW_LED_STATE_IGNORE, &action_sw_rf_on, 0);

	action_sw_rf_off.led_id = ledpriv->led_id;
	action_sw_rf_off.led_action = RTW_LED_ACTION_HIGH;
	rtw_led_register_event(padapter, RTW_LED_EVENT_SW_RF_OFF, RTW_LED_STATE_IGNORE, &action_sw_rf_off, 0);

	action_linked.led_id = ledpriv->led_id;
	action_linked.led_action = RTW_LED_ACTION_TOGGLE;
	action_linked.toggle_args.start_opt = RTW_LED_OPT_LOW;
	action_linked.toggle_args.start_delay = 0;
	action_linked.toggle_args.loop = 0;
	action_linked.toggle_args.intervals_idx = 0;
	rtw_led_register_event(padapter, RTW_LED_EVENT_LINKED, RTW_LED_STATE_IGNORE, &action_linked, RTW_LED_BLINK_INTERVAL);

#ifdef CONFIG_WIFI_LED_SHARE_WITH_WPS
	action_linked.led_id = ledpriv->led_id;
	action_linked.led_action = RTW_LED_ACTION_TOGGLE;
	action_linked.toggle_args.start_opt = RTW_LED_OPT_LOW;
	action_linked.toggle_args.start_delay = 0;
	action_linked.toggle_args.loop = 0;
	action_linked.toggle_args.intervals_idx = 0;
	rtw_led_register_event(padapter, RTW_LED_EVENT_WPS, RTW_LED_STATE_IGNORE, &action_linked, LED_BLINK_WPS_INTERVAL);
#endif

	/* register toggle intervals */
	rtw_led_set_toggle_intervals(padapter, 0, interval_arr, sizeof(interval_arr)/sizeof(u32));
}

int	rtw_init_led_priv(_adapter *padapter)
{
	struct led_priv *ledpriv = adapter_to_led(padapter);

#if defined(CONFIG_CONCURRENT_MODE)
	if (!is_primary_adapter(padapter))
		return -1;
#endif

	ledpriv->manual_ctrl = _FALSE;
	ledpriv->mode = LED_CTL_DOWN;
	ledpriv->led_id = RTW_LED_ID;
#ifdef CONFIG_WIFI_LED_SHARE_WITH_WPS
	ledpriv->last_mode = LED_CTL_DOWN;
#endif

	_rtw_led_module_init(padapter);

	return 0;
}
#endif /* CONFIG_RTW_SW_LED */
