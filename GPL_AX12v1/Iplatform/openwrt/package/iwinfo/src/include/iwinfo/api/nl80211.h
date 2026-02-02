#ifndef __LINUX_NL80211_H
#define __LINUX_NL80211_H
/*
 * 802.11 netlink interface public header
 *
 * Copyright 2006-2010 Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2008 Michael Wu <flamingice@sourmilk.net>
 * Copyright 2008 Luis Carlos Cobo <luisca@cozybit.com>
 * Copyright 2008 Michael Buesch <m@bues.ch>
 * Copyright 2008, 2009 Luis R. Rodriguez <lrodriguez@atheros.com>
 * Copyright 2008 Jouni Malinen <jouni.malinen@atheros.com>
 * Copyright 2008 Colin McCabe <colin@cozybit.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/types.h>

/**
 * DOC: Station handling
 *
 * Stations are added per interface, but a special case exists with VLAN
 * interfaces. When a station is bound to an AP interface, it may be moved
 * into a VLAN identified by a VLAN interface index (%NL80211_ATTR_STA_VLAN).
 * The station is still assumed to belong to the AP interface it was added
 * to.
 *
 * TODO: need more info?
 */

/**
 * DOC: Frame transmission/registration support
 *
 * Frame transmission and registration support exists to allow userspace
 * management entities such as wpa_supplicant react to management frames
 * that are not being handled by the kernel. This includes, for example,
 * certain classes of action frames that cannot be handled in the kernel
 * for various reasons.
 *
 * Frame registration is done on a per-interface basis and registrations
 * cannot be removed other than by closing the socket. It is possible to
 * specify a registration filter to register, for example, only for a
 * certain type of action frame. In particular with action frames, those
 * that userspace registers for will not be returned as unhandled by the
 * driver, so that the registered application has to take responsibility
 * for doing that.
 *
 * The type of frame that can be registered for is also dependent on the
 * driver and interface type. The frame types are advertised in wiphy
 * attributes so applications know what to expect.
 *
 * NOTE: When an interface changes type while registrations are active,
 *       these registrations are ignored until the interface type is
 *       changed again. This means that changing the interface type can
 *       lead to a situation that couldn't otherwise be produced, but
 *       any such registrations will be dormant in the sense that they
 *       will not be serviced, i.e. they will not receive any frames.
 *
 * Frame transmission allows userspace to send for example the required
 * responses to action frames. It is subject to some sanity checking,
 * but many frames can be transmitted. When a frame was transmitted, its
 * status is indicated to the sending socket.
 *
 * For more technical details, see the corresponding command descriptions
 * below.
 */

/**
 * DOC: Virtual interface / concurrency capabilities
 *
 * Some devices are able to operate with virtual MACs, they can have
 * more than one virtual interface. The capability handling for this
 * is a bit complex though, as there may be a number of restrictions
 * on the types of concurrency that are supported.
 *
 * To start with, each device supports the interface types listed in
 * the %NL80211_ATTR_SUPPORTED_IFTYPES attribute, but by listing the
 * types there no concurrency is implied.
 *
 * Once concurrency is desired, more attributes must be observed:
 * To start with, since some interface types are purely managed in
 * software, like the AP-VLAN type in mac80211 for example, there's
 * an additional list of these, they can be added at any time and
 * are only restricted by some semantic restrictions (e.g. AP-VLAN
 * cannot be added without a corresponding AP interface). This list
 * is exported in the %NL80211_ATTR_SOFTWARE_IFTYPES attribute.
 *
 * Further, the list of supported combinations is exported. This is
 * in the %NL80211_ATTR_INTERFACE_COMBINATIONS attribute. Basically,
 * it exports a list of "groups", and at any point in time the
 * interfaces that are currently active must fall into any one of
 * the advertised groups. Within each group, there are restrictions
 * on the number of interfaces of different types that are supported
 * and also the number of different channels, along with potentially
 * some other restrictions. See &enum nl80211_if_combination_attrs.
 *
 * All together, these attributes define the concurrency of virtual
 * interfaces that a given device supports.
 */

/**
 * enum nl80211_commands - supported nl80211 commands
 *
 * @NL80211_CMD_UNSPEC: unspecified command to catch errors
 *
 * @NL80211_CMD_GET_WIPHY: request information about a wiphy or dump request
 *	to get a list of all present wiphys.
 * @NL80211_CMD_SET_WIPHY: set wiphy parameters, needs %NL80211_ATTR_WIPHY or
 *	%NL80211_ATTR_IFINDEX; can be used to set %NL80211_ATTR_WIPHY_NAME,
 *	%NL80211_ATTR_WIPHY_TXQ_PARAMS, %NL80211_ATTR_WIPHY_FREQ,
 *	%NL80211_ATTR_WIPHY_CHANNEL_TYPE, %NL80211_ATTR_WIPHY_RETRY_SHORT,
 *	%NL80211_ATTR_WIPHY_RETRY_LONG, %NL80211_ATTR_WIPHY_FRAG_THRESHOLD,
 *	and/or %NL80211_ATTR_WIPHY_RTS_THRESHOLD.
 *	However, for setting the channel, see %NL80211_CMD_SET_CHANNEL
 *	instead, the support here is for backward compatibility only.
 * @NL80211_CMD_NEW_WIPHY: Newly created wiphy, response to get request
 *	or rename notification. Has attributes %NL80211_ATTR_WIPHY and
 *	%NL80211_ATTR_WIPHY_NAME.
 * @NL80211_CMD_DEL_WIPHY: Wiphy deleted. Has attributes
 *	%NL80211_ATTR_WIPHY and %NL80211_ATTR_WIPHY_NAME.
 *
 * @NL80211_CMD_GET_INTERFACE: Request an interface's configuration;
 *	either a dump request on a %NL80211_ATTR_WIPHY or a specific get
 *	on an %NL80211_ATTR_IFINDEX is supported.
 * @NL80211_CMD_SET_INTERFACE: Set type of a virtual interface, requires
 *	%NL80211_ATTR_IFINDEX and %NL80211_ATTR_IFTYPE.
 * @NL80211_CMD_NEW_INTERFACE: Newly created virtual interface or response
 *	to %NL80211_CMD_GET_INTERFACE. Has %NL80211_ATTR_IFINDEX,
 *	%NL80211_ATTR_WIPHY and %NL80211_ATTR_IFTYPE attributes. Can also
 *	be sent from userspace to request creation of a new virtual interface,
 *	then requires attributes %NL80211_ATTR_WIPHY, %NL80211_ATTR_IFTYPE and
 *	%NL80211_ATTR_IFNAME.
 * @NL80211_CMD_DEL_INTERFACE: Virtual interface was deleted, has attributes
 *	%NL80211_ATTR_IFINDEX and %NL80211_ATTR_WIPHY. Can also be sent from
 *	userspace to request deletion of a virtual interface, then requires
 *	attribute %NL80211_ATTR_IFINDEX.
 *
 * @NL80211_CMD_GET_KEY: Get sequence counter information for a key specified
 *	by %NL80211_ATTR_KEY_IDX and/or %NL80211_ATTR_MAC.
 * @NL80211_CMD_SET_KEY: Set key attributes %NL80211_ATTR_KEY_DEFAULT,
 *	%NL80211_ATTR_KEY_DEFAULT_MGMT, or %NL80211_ATTR_KEY_THRESHOLD.
 * @NL80211_CMD_NEW_KEY: add a key with given %NL80211_ATTR_KEY_DATA,
 *	%NL80211_ATTR_KEY_IDX, %NL80211_ATTR_MAC, %NL80211_ATTR_KEY_CIPHER,
 *	and %NL80211_ATTR_KEY_SEQ attributes.
 * @NL80211_CMD_DEL_KEY: delete a key identified by %NL80211_ATTR_KEY_IDX
 *	or %NL80211_ATTR_MAC.
 *
 * @NL80211_CMD_GET_BEACON: retrieve beacon information (returned in a
 *	%NL80222_CMD_NEW_BEACON message)
 * @NL80211_CMD_SET_BEACON: set the beacon on an access point interface
 *	using the %NL80211_ATTR_BEACON_INTERVAL, %NL80211_ATTR_DTIM_PERIOD,
 *	%NL80211_ATTR_BEACON_HEAD and %NL80211_ATTR_BEACON_TAIL attributes.
 *	Following attributes are provided for drivers that generate full Beacon
 *	and Probe Response frames internally: %NL80211_ATTR_SSID,
 *	%NL80211_ATTR_HIDDEN_SSID, %NL80211_ATTR_CIPHERS_PAIRWISE,
 *	%NL80211_ATTR_CIPHER_GROUP, %NL80211_ATTR_WPA_VERSIONS,
 *	%NL80211_ATTR_AKM_SUITES, %NL80211_ATTR_PRIVACY,
 *	%NL80211_ATTR_AUTH_TYPE, %NL80211_ATTR_IE, %NL80211_ATTR_IE_PROBE_RESP,
 *	%NL80211_ATTR_IE_ASSOC_RESP.
 * @NL80211_CMD_NEW_BEACON: add a new beacon to an access point interface,
 *	parameters are like for %NL80211_CMD_SET_BEACON.
 * @NL80211_CMD_DEL_BEACON: remove the beacon, stop sending it
 *
 * @NL80211_CMD_GET_STATION: Get station attributes for station identified by
 *	%NL80211_ATTR_MAC on the interface identified by %NL80211_ATTR_IFINDEX.
 * @NL80211_CMD_SET_STATION: Set station attributes for station identified by
 *	%NL80211_ATTR_MAC on the interface identified by %NL80211_ATTR_IFINDEX.
 * @NL80211_CMD_NEW_STATION: Add a station with given attributes to the
 *	the interface identified by %NL80211_ATTR_IFINDEX.
 * @NL80211_CMD_DEL_STATION: Remove a station identified by %NL80211_ATTR_MAC
 *	or, if no MAC address given, all stations, on the interface identified
 *	by %NL80211_ATTR_IFINDEX.
 *
 * @NL80211_CMD_GET_MPATH: Get mesh path attributes for mesh path to
 * 	destination %NL80211_ATTR_MAC on the interface identified by
 * 	%NL80211_ATTR_IFINDEX.
 * @NL80211_CMD_SET_MPATH:  Set mesh path attributes for mesh path to
 * 	destination %NL80211_ATTR_MAC on the interface identified by
 * 	%NL80211_ATTR_IFINDEX.
 * @NL80211_CMD_NEW_MPATH: Create a new mesh path for the destination given by
 *	%NL80211_ATTR_MAC via %NL80211_ATTR_MPATH_NEXT_HOP.
 * @NL80211_CMD_DEL_MPATH: Delete a mesh path to the destination given by
 *	%NL80211_ATTR_MAC.
 * @NL80211_CMD_NEW_PATH: Add a mesh path with given attributes to the
 *	the interface identified by %NL80211_ATTR_IFINDEX.
 * @NL80211_CMD_DEL_PATH: Remove a mesh path identified by %NL80211_ATTR_MAC
 *	or, if no MAC address given, all mesh paths, on the interface identified
 *	by %NL80211_ATTR_IFINDEX.
 * @NL80211_CMD_SET_BSS: Set BSS attributes for BSS identified by
 *	%NL80211_ATTR_IFINDEX.
 *
 * @NL80211_CMD_GET_REG: ask the wireless core to send us its currently set
 * 	regulatory domain.
 * @NL80211_CMD_SET_REG: Set current regulatory domain. CRDA sends this command
 *	after being queried by the kernel. CRDA replies by sending a regulatory
 *	domain structure which consists of %NL80211_ATTR_REG_ALPHA set to our
 *	current alpha2 if it found a match. It also provides
 * 	NL80211_ATTR_REG_RULE_FLAGS, and a set of regulatory rules. Each
 * 	regulatory rule is a nested set of attributes  given by
 * 	%NL80211_ATTR_REG_RULE_FREQ_[START|END] and
 * 	%NL80211_ATTR_FREQ_RANGE_MAX_BW with an attached power rule given by
 * 	%NL80211_ATTR_REG_RULE_POWER_MAX_ANT_GAIN and
 * 	%NL80211_ATTR_REG_RULE_POWER_MAX_EIRP.
 * @NL80211_CMD_REQ_SET_REG: ask the wireless core to set the regulatory domain
 * 	to the specified ISO/IEC 3166-1 alpha2 country code. The core will
 * 	store this as a valid request and then query userspace for it.
 *
 * @NL80211_CMD_GET_MESH_CONFIG: Get mesh networking properties for the
 *	interface identified by %NL80211_ATTR_IFINDEX
 *
 * @NL80211_CMD_SET_MESH_CONFIG: Set mesh networking properties for the
 *      interface identified by %NL80211_ATTR_IFINDEX
 *
 * @NL80211_CMD_SET_MGMT_EXTRA_IE: Set extra IEs for management frames. The
 *	interface is identified with %NL80211_ATTR_IFINDEX and the management
 *	frame subtype with %NL80211_ATTR_MGMT_SUBTYPE. The extra IE data to be
 *	added to the end of the specified management frame is specified with
 *	%NL80211_ATTR_IE. If the command succeeds, the requested data will be
 *	added to all specified management frames generated by
 *	kernel/firmware/driver.
 *	Note: This command has been removed and it is only reserved at this
 *	point to avoid re-using existing command number. The functionality this
 *	command was planned for has been provided with cleaner design with the
 *	option to specify additional IEs in NL80211_CMD_TRIGGER_SCAN,
 *	NL80211_CMD_AUTHENTICATE, NL80211_CMD_ASSOCIATE,
 *	NL80211_CMD_DEAUTHENTICATE, and NL80211_CMD_DISASSOCIATE.
 *
 * @NL80211_CMD_GET_SCAN: get scan results
 * @NL80211_CMD_TRIGGER_SCAN: trigger a new scan with the given parameters
 *	%NL80211_ATTR_TX_NO_CCK_RATE is used to decide whether to send the
 *	probe requests at CCK rate or not.
 * @NL80211_CMD_NEW_SCAN_RESULTS: scan notification (as a reply to
 *	NL80211_CMD_GET_SCAN and on the "scan" multicast group)
 * @NL80211_CMD_SCAN_ABORTED: scan was aborted, for unspecified reasons,
 *	partial scan results may be available
 *
 * @NL80211_CMD_START_SCHED_SCAN: start a scheduled scan at certain
 *	intervals, as specified by %NL80211_ATTR_SCHED_SCAN_INTERVAL.
 *	Like with normal scans, if SSIDs (%NL80211_ATTR_SCAN_SSIDS)
 *	are passed, they are used in the probe requests.  For
 *	broadcast, a broadcast SSID must be passed (ie. an empty
 *	string).  If no SSID is passed, no probe requests are sent and
 *	a passive scan is performed.  %NL80211_ATTR_SCAN_FREQUENCIES,
 *	if passed, define which channels should be scanned; if not
 *	passed, all channels allowed for the current regulatory domain
 *	are used.  Extra IEs can also be passed from the userspace by
 *	using the %NL80211_ATTR_IE attribute.
 * @NL80211_CMD_STOP_SCHED_SCAN: stop a scheduled scan.  Returns -ENOENT
 *	if scheduled scan is not running.
 * @NL80211_CMD_SCHED_SCAN_RESULTS: indicates that there are scheduled scan
 *	results available.
 * @NL80211_CMD_SCHED_SCAN_STOPPED: indicates that the scheduled scan has
 *	stopped.  The driver may issue this event at any time during a
 *	scheduled scan.  One reason for stopping the scan is if the hardware
 *	does not support starting an association or a normal scan while running
 *	a scheduled scan.  This event is also sent when the
 *	%NL80211_CMD_STOP_SCHED_SCAN command is received or when the interface
 *	is brought down while a scheduled scan was running.
 *
 * @NL80211_CMD_GET_SURVEY: get survey resuls, e.g. channel occupation
 *      or noise level
 * @NL80211_CMD_NEW_SURVEY_RESULTS: survey data notification (as a reply to
 *	NL80211_CMD_GET_SURVEY and on the "scan" multicast group)
 *
 * @NL80211_CMD_REG_CHANGE: indicates to userspace the regulatory domain
 * 	has been changed and provides details of the request information
 * 	that caused the change such as who initiated the regulatory request
 * 	(%NL80211_ATTR_REG_INITIATOR), the wiphy_idx
 * 	(%NL80211_ATTR_REG_ALPHA2) on which the request was made from if
 * 	the initiator was %NL80211_REGDOM_SET_BY_COUNTRY_IE or
 * 	%NL80211_REGDOM_SET_BY_DRIVER, the type of regulatory domain
 * 	set (%NL80211_ATTR_REG_TYPE), if the type of regulatory domain is
 * 	%NL80211_REG_TYPE_COUNTRY the alpha2 to which we have moved on
 * 	to (%NL80211_ATTR_REG_ALPHA2).
 * @NL80211_CMD_REG_BEACON_HINT: indicates to userspace that an AP beacon
 * 	has been found while world roaming thus enabling active scan or
 * 	any mode of operation that initiates TX (beacons) on a channel
 * 	where we would not have been able to do either before. As an example
 * 	if you are world roaming (regulatory domain set to world or if your
 * 	driver is using a custom world roaming regulatory domain) and while
 * 	doing a passive scan on the 5 GHz band you find an AP there (if not
 * 	on a DFS channel) you will now be able to actively scan for that AP
 * 	or use AP mode on your card on that same channel. Note that this will
 * 	never be used for channels 1-11 on the 2 GHz band as they are always
 * 	enabled world wide. This beacon hint is only sent if your device had
 * 	either disabled active scanning or beaconing on a channel. We send to
 * 	userspace the wiphy on which we removed a restriction from
 * 	(%NL80211_ATTR_WIPHY) and the channel on which this occurred
 * 	before (%NL80211_ATTR_FREQ_BEFORE) and after (%NL80211_ATTR_FREQ_AFTER)
 * 	the beacon hint was processed.
 *
 * @NL80211_CMD_AUTHENTICATE: authentication request and notification.
 *	This command is used both as a command (request to authenticate) and
 *	as an event on the "mlme" multicast group indicating completion of the
 *	authentication process.
 *	When used as a command, %NL80211_ATTR_IFINDEX is used to identify the
 *	interface. %NL80211_ATTR_MAC is used to specify PeerSTAAddress (and
 *	BSSID in case of station mode). %NL80211_ATTR_SSID is used to specify
 *	the SSID (mainly for association, but is included in authentication
 *	request, too, to help BSS selection. %NL80211_ATTR_WIPHY_FREQ is used
 *	to specify the frequence of the channel in MHz. %NL80211_ATTR_AUTH_TYPE
 *	is used to specify the authentication type. %NL80211_ATTR_IE is used to
 *	define IEs (VendorSpecificInfo, but also including RSN IE and FT IEs)
 *	to be added to the frame.
 *	When used as an event, this reports reception of an Authentication
 *	frame in station and IBSS modes when the local MLME processed the
 *	frame, i.e., it was for the local STA and was received in correct
 *	state. This is similar to MLME-AUTHENTICATE.confirm primitive in the
 *	MLME SAP interface (kernel providing MLME, userspace SME). The
 *	included %NL80211_ATTR_FRAME attribute contains the management frame
 *	(including both the header and frame body, but not FCS). This event is
 *	also used to indicate if the authentication attempt timed out. In that
 *	case the %NL80211_ATTR_FRAME attribute is replaced with a
 *	%NL80211_ATTR_TIMED_OUT flag (and %NL80211_ATTR_MAC to indicate which
 *	pending authentication timed out).
 * @NL80211_CMD_ASSOCIATE: association request and notification; like
 *	NL80211_CMD_AUTHENTICATE but for Association and Reassociation
 *	(similar to MLME-ASSOCIATE.request, MLME-REASSOCIATE.request,
 *	MLME-ASSOCIATE.confirm or MLME-REASSOCIATE.confirm primitives).
 * @NL80211_CMD_DEAUTHENTICATE: deauthentication request and notification; like
 *	NL80211_CMD_AUTHENTICATE but for Deauthentication frames (similar to
 *	MLME-DEAUTHENTICATION.request and MLME-DEAUTHENTICATE.indication
 *	primitives).
 * @NL80211_CMD_DISASSOCIATE: disassociation request and notification; like
 *	NL80211_CMD_AUTHENTICATE but for Disassociation frames (similar to
 *	MLME-DISASSOCIATE.request and MLME-DISASSOCIATE.indication primitives).
 *
 * @NL80211_CMD_MICHAEL_MIC_FAILURE: notification of a locally detected Michael
 *	MIC (part of TKIP) failure; sent on the "mlme" multicast group; the
 *	event includes %NL80211_ATTR_MAC to describe the source MAC address of
 *	the frame with invalid MIC, %NL80211_ATTR_KEY_TYPE to show the key
 *	type, %NL80211_ATTR_KEY_IDX to indicate the key identifier, and
 *	%NL80211_ATTR_KEY_SEQ to indicate the TSC value of the frame; this
 *	event matches with MLME-MICHAELMICFAILURE.indication() primitive
 *
 * @NL80211_CMD_JOIN_IBSS: Join a new IBSS -- given at least an SSID and a
 *	FREQ attribute (for the initial frequency if no peer can be found)
 *	and optionally a MAC (as BSSID) and FREQ_FIXED attribute if those
 *	should be fixed rather than automatically determined. Can only be
 *	executed on a network interface that is UP, and fixed BSSID/FREQ
 *	may be rejected. Another optional parameter is the beacon interval,
 *	given in the %NL80211_ATTR_BEACON_INTERVAL attribute, which if not
 *	given defaults to 100 TU (102.4ms).
 * @NL80211_CMD_LEAVE_IBSS: Leave the IBSS -- no special arguments, the IBSS is
 *	determined by the network interface.
 *
 * @NL80211_CMD_TESTMODE: testmode command, takes a wiphy (or ifindex) attribute
 *	to identify the device, and the TESTDATA blob attribute to pass through
 *	to the driver.
 *
 * @NL80211_CMD_CONNECT: connection request and notification; this command
 *	requests to connect to a specified network but without separating
 *	auth and assoc steps. For this, you need to specify the SSID in a
 *	%NL80211_ATTR_SSID attribute, and can optionally specify the association
 *	IEs in %NL80211_ATTR_IE, %NL80211_ATTR_AUTH_TYPE, %NL80211_ATTR_MAC,
 *	%NL80211_ATTR_WIPHY_FREQ, %NL80211_ATTR_CONTROL_PORT,
 *	%NL80211_ATTR_CONTROL_PORT_ETHERTYPE and
 *	%NL80211_ATTR_CONTROL_PORT_NO_ENCRYPT.
 *	It is also sent as an event, with the BSSID and response IEs when the
 *	connection is established or failed to be established. This can be
 *	determined by the STATUS_CODE attribute.
 * @NL80211_CMD_ROAM: request that the card roam (currently not implemented),
 *	sent as an event when the card/driver roamed by itself.
 * @NL80211_CMD_DISCONNECT: drop a given connection; also used to notify
 *	userspace that a connection was dropped by the AP or due to other
 *	reasons, for this the %NL80211_ATTR_DISCONNECTED_BY_AP and
 *	%NL80211_ATTR_REASON_CODE attributes are used.
 *
 * @NL80211_CMD_SET_WIPHY_NETNS: Set a wiphy's netns. Note that all devices
 *	associated with this wiphy must be down and will follow.
 *
 * @NL80211_CMD_REMAIN_ON_CHANNEL: Request to remain awake on the specified
 *	channel for the specified amount of time. This can be used to do
 *	off-channel operations like transmit a Public Action frame and wait for
 *	a response while being associated to an AP on another channel.
 *	%NL80211_ATTR_IFINDEX is used to specify which interface (and thus
 *	radio) is used. %NL80211_ATTR_WIPHY_FREQ is used to specify the
 *	frequency for the operation and %NL80211_ATTR_WIPHY_CHANNEL_TYPE may be
 *	optionally used to specify additional channel parameters.
 *	%NL80211_ATTR_DURATION is used to specify the duration in milliseconds
 *	to remain on the channel. This command is also used as an event to
 *	notify when the requested duration starts (it may take a while for the
 *	driver to schedule this time due to other concurrent needs for the
 *	radio).
 *	When called, this operation returns a cookie (%NL80211_ATTR_COOKIE)
 *	that will be included with any events pertaining to this request;
 *	the cookie is also used to cancel the request.
 * @NL80211_CMD_CANCEL_REMAIN_ON_CHANNEL: This command can be used to cancel a
 *	pending remain-on-channel duration if the desired operation has been
 *	completed prior to expiration of the originally requested duration.
 *	%NL80211_ATTR_WIPHY or %NL80211_ATTR_IFINDEX is used to specify the
 *	radio. The %NL80211_ATTR_COOKIE attribute must be given as well to
 *	uniquely identify the request.
 *	This command is also used as an event to notify when a requested
 *	remain-on-channel duration has expired.
 *
 * @NL80211_CMD_SET_TX_BITRATE_MASK: Set the mask of rates to be used in TX
 *	rate selection. %NL80211_ATTR_IFINDEX is used to specify the interface
 *	and @NL80211_ATTR_TX_RATES the set of allowed rates.
 *
 * @NL80211_CMD_REGISTER_FRAME: Register for receiving certain mgmt frames
 *	(via @NL80211_CMD_FRAME) for processing in userspace. This command
 *	requires an interface index, a frame type attribute (optional for
 *	backward compatibility reasons, if not given assumes action frames)
 *	and a match attribute containing the first few bytes of the frame
 *	that should match, e.g. a single byte for only a category match or
 *	four bytes for vendor frames including the OUI. The registration
 *	cannot be dropped, but is removed automatically when the netlink
 *	socket is closed. Multiple registrations can be made.
 * @NL80211_CMD_REGISTER_ACTION: Alias for @NL80211_CMD_REGISTER_FRAME for
 *	backward compatibility
 * @NL80211_CMD_FRAME: Management frame TX request and RX notification. This
 *	command is used both as a request to transmit a management frame and
 *	as an event indicating reception of a frame that was not processed in
 *	kernel code, but is for us (i.e., which may need to be processed in a
 *	user space application). %NL80211_ATTR_FRAME is used to specify the
 *	frame contents (including header). %NL80211_ATTR_WIPHY_FREQ (and
 *	optionally %NL80211_ATTR_WIPHY_CHANNEL_TYPE) is used to indicate on
 *	which channel the frame is to be transmitted or was received. If this
 *	channel is not the current channel (remain-on-channel or the
 *	operational channel) the device will switch to the given channel and
 *	transmit the frame, optionally waiting for a response for the time
 *	specified using %NL80211_ATTR_DURATION. When called, this operation
 *	returns a cookie (%NL80211_ATTR_COOKIE) that will be included with the
 *	TX status event pertaining to the TX request.
 *	%NL80211_ATTR_TX_NO_CCK_RATE is used to decide whether to send the
 *	management frames at CCK rate or not in 2GHz band.
 * @NL80211_CMD_FRAME_WAIT_CANCEL: When an off-channel TX was requested, this
 *	command may be used with the corresponding cookie to cancel the wait
 *	time if it is known that it is no longer necessary.
 * @NL80211_CMD_ACTION: Alias for @NL80211_CMD_FRAME for backward compatibility.
 * @NL80211_CMD_FRAME_TX_STATUS: Report TX status of a management frame
 *	transmitted with %NL80211_CMD_FRAME. %NL80211_ATTR_COOKIE identifies
 *	the TX command and %NL80211_ATTR_FRAME includes the contents of the
 *	frame. %NL80211_ATTR_ACK flag is included if the recipient acknowledged
 *	the frame.
 * @NL80211_CMD_ACTION_TX_STATUS: Alias for @NL80211_CMD_FRAME_TX_STATUS for
 *	backward compatibility.
 * @NL80211_CMD_SET_CQM: Connection quality monitor configuration. This command
 *	is used to configure connection quality monitoring notification trigger
 *	levels.
 * @NL80211_CMD_NOTIFY_CQM: Connection quality monitor notification. This
 *	command is used as an event to indicate the that a trigger level was
 *	reached.
 * @NL80211_CMD_SET_CHANNEL: Set the channel (using %NL80211_ATTR_WIPHY_FREQ
 *	and %NL80211_ATTR_WIPHY_CHANNEL_TYPE) the given interface (identifed
 *	by %NL80211_ATTR_IFINDEX) shall operate on.
 *	In case multiple channels are supported by the device, the mechanism
 *	with which it switches channels is implementation-defined.
 *	When a monitor interface is given, it can only switch channel while
 *	no other interfaces are operating to avoid disturbing the operation
 *	of any other interfaces, and other interfaces will again take
 *	precedence when they are used.
 *
 * @NL80211_CMD_SET_WDS_PEER: Set the MAC address of the peer on a WDS interface.
 *
 * @NL80211_CMD_JOIN_MESH: Join a mesh. The mesh ID must be given, and initial
 *	mesh config parameters may be given.
 * @NL80211_CMD_LEAVE_MESH: Leave the mesh network -- no special arguments, the
 *	network is determined by the network interface.
 *
 * @NL80211_CMD_UNPROT_DEAUTHENTICATE: Unprotected deauthentication frame
 *	notification. This event is used to indicate that an unprotected
 *	deauthentication frame was dropped when MFP is in use.
 * @NL80211_CMD_UNPROT_DISASSOCIATE: Unprotected disassociation frame
 *	notification. This event is used to indicate that an unprotected
 *	disassociation frame was dropped when MFP is in use.
 *
 * @NL80211_CMD_NEW_PEER_CANDIDATE: Notification on the reception of a
 *      beacon or probe response from a compatible mesh peer.  This is only
 *      sent while no station information (sta_info) exists for the new peer
 *      candidate and when @NL80211_MESH_SETUP_USERSPACE_AUTH is set.  On
 *      reception of this notification, userspace may decide to create a new
 *      station (@NL80211_CMD_NEW_STATION).  To stop this notification from
 *      reoccurring, the userspace authentication daemon may want to create the
 *      new station with the AUTHENTICATED flag unset and maybe change it later
 *      depending on the authentication result.
 *
 * @NL80211_CMD_GET_WOWLAN: get Wake-on-Wireless-LAN (WoWLAN) settings.
 * @NL80211_CMD_SET_WOWLAN: set Wake-on-Wireless-LAN (WoWLAN) settings.
 *	Since wireless is more complex than wired ethernet, it supports
 *	various triggers. These triggers can be configured through this
 *	command with the %NL80211_ATTR_WOWLAN_TRIGGERS attribute. For
 *	more background information, see
 *	http://wireless.kernel.org/en/users/Documentation/WoWLAN.
 *
 * @NL80211_CMD_SET_REKEY_OFFLOAD: This command is used give the driver
 *	the necessary information for supporting GTK rekey offload. This
 *	feature is typically used during WoWLAN. The configuration data
 *	is contained in %NL80211_ATTR_REKEY_DATA (which is nested and
 *	contains the data in sub-attributes). After rekeying happened,
 *	this command may also be sent by the driver as an MLME event to
 *	inform userspace of the new replay counter.
 *
 * @NL80211_CMD_PMKSA_CANDIDATE: This is used as an event to inform userspace
 *	of PMKSA caching dandidates.
 *
 * @NL80211_CMD_TDLS_OPER: Perform a high-level TDLS command (e.g. link setup).
 * @NL80211_CMD_TDLS_MGMT: Send a TDLS management frame.
 *
 * @NL80211_CMD_UNEXPECTED_FRAME: Used by an application controlling an AP
 *	(or GO) interface (i.e. hostapd) to ask for unexpected frames to
 *	implement sending deauth to stations that send unexpected class 3
 *	frames. Also used as the event sent by the kernel when such a frame
 *	is received.
 *	For the event, the %NL80211_ATTR_MAC attribute carries the TA and
 *	other attributes like the interface index are present.
 *	If used as the command it must have an interface index and you can
 *	only unsubscribe from the event by closing the socket. Subscription
 *	is also for %NL80211_CMD_UNEXPECTED_4ADDR_FRAME events.
 *
 * @NL80211_CMD_UNEXPECTED_4ADDR_FRAME: Sent as an event indicating that the
 *	associated station identified by %NL80211_ATTR_MAC sent a 4addr frame
 *	and wasn't already in a 4-addr VLAN. The event will be sent similarly
 *	to the %NL80211_CMD_UNEXPECTED_FRAME event, to the same listener.
 *
 * @NL80211_CMD_PROBE_CLIENT: Probe an associated station on an AP interface
 *	by sending a null data frame to it and reporting when the frame is
 *	acknowleged. This is used to allow timing out inactive clients. Uses
 *	%NL80211_ATTR_IFINDEX and %NL80211_ATTR_MAC. The command returns a
 *	direct reply with an %NL80211_ATTR_COOKIE that is later used to match
 *	up the event with the request. The event includes the same data and
 *	has %NL80211_ATTR_ACK set if the frame was ACKed.
 *
 * @NL80211_CMD_REGISTER_BEACONS: Register this socket to receive beacons from
 *	other BSSes when any interfaces are in AP mode. This helps implement
 *	OLBC handling in hostapd. Beacons are reported in %NL80211_CMD_FRAME
 *	messages. Note that per PHY only one application may register.
 *
 * @NL80211_CMD_SET_NOACK_MAP: sets a bitmap for the individual TIDs whether
 *      No Acknowledgement Policy should be applied.
 *
 * @NL80211_CMD_MAX: highest used command number
 * @__NL80211_CMD_AFTER_LAST: internal use
 */
enum nl80211_commands {
/* don't change the order or add anything between, this is ABI! */
	NL80211_CMD_UNSPEC,

	NL80211_CMD_GET_WIPHY,		/* can dump */
	NL80211_CMD_SET_WIPHY,
	NL80211_CMD_NEW_WIPHY,
	NL80211_CMD_DEL_WIPHY,

	NL80211_CMD_GET_INTERFACE,	/* can dump */
	NL80211_CMD_SET_INTERFACE,
	NL80211_CMD_NEW_INTERFACE,
	NL80211_CMD_DEL_INTERFACE,

	NL80211_CMD_GET_KEY,
	NL80211_CMD_SET_KEY,
	NL80211_CMD_NEW_KEY,
	NL80211_CMD_DEL_KEY,

	NL80211_CMD_GET_BEACON,
	NL80211_CMD_SET_BEACON,
	NL80211_CMD_NEW_BEACON,
	NL80211_CMD_DEL_BEACON,

	NL80211_CMD_GET_STATION,
	NL80211_CMD_SET_STATION,
	NL80211_CMD_NEW_STATION,
	NL80211_CMD_DEL_STATION,

	NL80211_CMD_GET_MPATH,
	NL80211_CMD_SET_MPATH,
	NL80211_CMD_NEW_MPATH,
	NL80211_CMD_DEL_MPATH,

	NL80211_CMD_SET_BSS,

	NL80211_CMD_SET_REG,
	NL80211_CMD_REQ_SET_REG,

	NL80211_CMD_GET_MESH_CONFIG,
	NL80211_CMD_SET_MESH_CONFIG,

	NL80211_CMD_SET_MGMT_EXTRA_IE /* reserved; not used */,

	NL80211_CMD_GET_REG,

	NL80211_CMD_GET_SCAN,
	NL80211_CMD_TRIGGER_SCAN,
	NL80211_CMD_NEW_SCAN_RESULTS,
	NL80211_CMD_SCAN_ABORTED,

	NL80211_CMD_REG_CHANGE,

	NL80211_CMD_AUTHENTICATE,
	NL80211_CMD_ASSOCIATE,
	NL80211_CMD_DEAUTHENTICATE,
	NL80211_CMD_DISASSOCIATE,

	NL80211_CMD_MICHAEL_MIC_FAILURE,

	NL80211_CMD_REG_BEACON_HINT,

	NL80211_CMD_JOIN_IBSS,
	NL80211_CMD_LEAVE_IBSS,

	NL80211_CMD_TESTMODE,

	NL80211_CMD_CONNECT,
	NL80211_CMD_ROAM,
	NL80211_CMD_DISCONNECT,

	NL80211_CMD_SET_WIPHY_NETNS,

	NL80211_CMD_GET_SURVEY,
	NL80211_CMD_NEW_SURVEY_RESULTS,

	NL80211_CMD_SET_PMKSA,
	NL80211_CMD_DEL_PMKSA,
	NL80211_CMD_FLUSH_PMKSA,

	NL80211_CMD_REMAIN_ON_CHANNEL,
	NL80211_CMD_CANCEL_REMAIN_ON_CHANNEL,

	NL80211_CMD_SET_TX_BITRATE_MASK,

	NL80211_CMD_REGISTER_FRAME,
	NL80211_CMD_REGISTER_ACTION = NL80211_CMD_REGISTER_FRAME,
	NL80211_CMD_FRAME,
	NL80211_CMD_ACTION = NL80211_CMD_FRAME,
	NL80211_CMD_FRAME_TX_STATUS,
	NL80211_CMD_ACTION_TX_STATUS = NL80211_CMD_FRAME_TX_STATUS,

	NL80211_CMD_SET_POWER_SAVE,
	NL80211_CMD_GET_POWER_SAVE,

	NL80211_CMD_SET_CQM,
	NL80211_CMD_NOTIFY_CQM,

	NL80211_CMD_SET_CHANNEL,
	NL80211_CMD_SET_WDS_PEER,

	NL80211_CMD_FRAME_WAIT_CANCEL,

	NL80211_CMD_JOIN_MESH,
	NL80211_CMD_LEAVE_MESH,

	NL80211_CMD_UNPROT_DEAUTHENTICATE,
	NL80211_CMD_UNPROT_DISASSOCIATE,

	NL80211_CMD_NEW_PEER_CANDIDATE,

	NL80211_CMD_GET_WOWLAN,
	NL80211_CMD_SET_WOWLAN,

	NL80211_CMD_START_SCHED_SCAN,
	NL80211_CMD_STOP_SCHED_SCAN,
	NL80211_CMD_SCHED_SCAN_RESULTS,
	NL80211_CMD_SCHED_SCAN_STOPPED,

	NL80211_CMD_SET_REKEY_OFFLOAD,

	NL80211_CMD_PMKSA_CANDIDATE,

	NL80211_CMD_TDLS_OPER,
	NL80211_CMD_TDLS_MGMT,

	NL80211_CMD_UNEXPECTED_FRAME,

	NL80211_CMD_PROBE_CLIENT,

	NL80211_CMD_REGISTER_BEACONS,

	NL80211_CMD_UNEXPECTED_4ADDR_FRAME,

	NL80211_CMD_SET_NOACK_MAP,

	NL80211_CMD_CH_SWITCH_NOTIFY,

	NL80211_CMD_START_P2P_DEVICE,
	NL80211_CMD_STOP_P2P_DEVICE,

	NL80211_CMD_CONN_FAILED,

	NL80211_CMD_SET_MCAST_RATE,

	NL80211_CMD_SET_MAC_ACL,

	NL80211_CMD_RADAR_DETECT,

	NL80211_CMD_GET_PROTOCOL_FEATURES,

	NL80211_CMD_UPDATE_FT_IES,
	NL80211_CMD_FT_EVENT,

	NL80211_CMD_CRIT_PROTOCOL_START,
	NL80211_CMD_CRIT_PROTOCOL_STOP,

	NL80211_CMD_GET_COALESCE,
	NL80211_CMD_SET_COALESCE,

	NL80211_CMD_CHANNEL_SWITCH,

	NL80211_CMD_VENDOR,

	NL80211_CMD_SET_QOS_MAP,

	NL80211_CMD_ADD_TX_TS,
	NL80211_CMD_DEL_TX_TS,

	NL80211_CMD_GET_MPP,

	NL80211_CMD_JOIN_OCB,
	NL80211_CMD_LEAVE_OCB,

	NL80211_CMD_CH_SWITCH_STARTED_NOTIFY,

	NL80211_CMD_TDLS_CHANNEL_SWITCH,
	NL80211_CMD_TDLS_CANCEL_CHANNEL_SWITCH,

	NL80211_CMD_WIPHY_REG_CHANGE,

	NL80211_CMD_ABORT_SCAN,

	NL80211_CMD_START_NAN,
	NL80211_CMD_STOP_NAN,
	NL80211_CMD_ADD_NAN_FUNCTION,
	NL80211_CMD_DEL_NAN_FUNCTION,
	NL80211_CMD_CHANGE_NAN_CONFIG,
	NL80211_CMD_NAN_MATCH,

	NL80211_CMD_SET_MULTICAST_TO_UNICAST,

	NL80211_CMD_UPDATE_CONNECT_PARAMS,

	NL80211_CMD_SET_PMK,
	NL80211_CMD_DEL_PMK,

	NL80211_CMD_PORT_AUTHORIZED,

	NL80211_CMD_RELOAD_REGDB,

	NL80211_CMD_EXTERNAL_AUTH,

	NL80211_CMD_STA_OPMODE_CHANGED,

	NL80211_CMD_CONTROL_PORT_FRAME,

	NL80211_CMD_GET_FTM_RESPONDER_STATS,

	NL80211_CMD_PEER_MEASUREMENT_START,
	NL80211_CMD_PEER_MEASUREMENT_RESULT,
	NL80211_CMD_PEER_MEASUREMENT_COMPLETE,

	NL80211_CMD_NOTIFY_RADAR,

	NL80211_CMD_UPDATE_OWE_INFO,

	NL80211_CMD_PROBE_MESH_LINK,

	NL80211_CMD_SET_FILS_AAD,

	/* add new commands above here */

	/* used to define NL80211_CMD_MAX below */
	__NL80211_CMD_AFTER_LAST,
	NL80211_CMD_MAX = __NL80211_CMD_AFTER_LAST - 1
};

/*
 * Allow user space programs to use #ifdef on new commands by defining them
 * here
 */
#define NL80211_CMD_SET_BSS NL80211_CMD_SET_BSS
#define NL80211_CMD_SET_MGMT_EXTRA_IE NL80211_CMD_SET_MGMT_EXTRA_IE
#define NL80211_CMD_REG_CHANGE NL80211_CMD_REG_CHANGE
#define NL80211_CMD_AUTHENTICATE NL80211_CMD_AUTHENTICATE
#define NL80211_CMD_ASSOCIATE NL80211_CMD_ASSOCIATE
#define NL80211_CMD_DEAUTHENTICATE NL80211_CMD_DEAUTHENTICATE
#define NL80211_CMD_DISASSOCIATE NL80211_CMD_DISASSOCIATE
#define NL80211_CMD_REG_BEACON_HINT NL80211_CMD_REG_BEACON_HINT

#define NL80211_ATTR_FEATURE_FLAGS NL80211_ATTR_FEATURE_FLAGS

/* source-level API compatibility */
#define NL80211_CMD_GET_MESH_PARAMS NL80211_CMD_GET_MESH_CONFIG
#define NL80211_CMD_SET_MESH_PARAMS NL80211_CMD_SET_MESH_CONFIG
#define NL80211_MESH_SETUP_VENDOR_PATH_SEL_IE NL80211_MESH_SETUP_IE

/**
 * enum nl80211_attrs - nl80211 netlink attributes
 *
 * @NL80211_ATTR_UNSPEC: unspecified attribute to catch errors
 *
 * @NL80211_ATTR_WIPHY: index of wiphy to operate on, cf.
 *	/sys/class/ieee80211/<phyname>/index
 * @NL80211_ATTR_WIPHY_NAME: wiphy name (used for renaming)
 * @NL80211_ATTR_WIPHY_TXQ_PARAMS: a nested array of TX queue parameters
 * @NL80211_ATTR_WIPHY_FREQ: frequency of the selected channel in MHz
 * @NL80211_ATTR_WIPHY_CHANNEL_TYPE: included with NL80211_ATTR_WIPHY_FREQ
 *	if HT20 or HT40 are allowed (i.e., 802.11n disabled if not included):
 *	NL80211_CHAN_NO_HT = HT not allowed (i.e., same as not including
 *		this attribute)
 *	NL80211_CHAN_HT20 = HT20 only
 *	NL80211_CHAN_HT40MINUS = secondary channel is below the primary channel
 *	NL80211_CHAN_HT40PLUS = secondary channel is above the primary channel
 * @NL80211_ATTR_WIPHY_RETRY_SHORT: TX retry limit for frames whose length is
 *	less than or equal to the RTS threshold; allowed range: 1..255;
 *	dot11ShortRetryLimit; u8
 * @NL80211_ATTR_WIPHY_RETRY_LONG: TX retry limit for frames whose length is
 *	greater than the RTS threshold; allowed range: 1..255;
 *	dot11ShortLongLimit; u8
 * @NL80211_ATTR_WIPHY_FRAG_THRESHOLD: fragmentation threshold, i.e., maximum
 *	length in octets for frames; allowed range: 256..8000, disable
 *	fragmentation with (u32)-1; dot11FragmentationThreshold; u32
 * @NL80211_ATTR_WIPHY_RTS_THRESHOLD: RTS threshold (TX frames with length
 *	larger than or equal to this use RTS/CTS handshake); allowed range:
 *	0..65536, disable with (u32)-1; dot11RTSThreshold; u32
 * @NL80211_ATTR_WIPHY_COVERAGE_CLASS: Coverage Class as defined by IEEE 802.11
 *	section 7.3.2.9; dot11CoverageClass; u8
 *
 * @NL80211_ATTR_IFINDEX: network interface index of the device to operate on
 * @NL80211_ATTR_IFNAME: network interface name
 * @NL80211_ATTR_IFTYPE: type of virtual interface, see &enum nl80211_iftype
 *
 * @NL80211_ATTR_MAC: MAC address (various uses)
 *
 * @NL80211_ATTR_KEY_DATA: (temporal) key data; for TKIP this consists of
 *	16 bytes encryption key followed by 8 bytes each for TX and RX MIC
 *	keys
 * @NL80211_ATTR_KEY_IDX: key ID (u8, 0-3)
 * @NL80211_ATTR_KEY_CIPHER: key cipher suite (u32, as defined by IEEE 802.11
 *	section 7.3.2.25.1, e.g. 0x000FAC04)
 * @NL80211_ATTR_KEY_SEQ: transmit key sequence number (IV/PN) for TKIP and
 *	CCMP keys, each six bytes in little endian
 *
 * @NL80211_ATTR_BEACON_INTERVAL: beacon interval in TU
 * @NL80211_ATTR_DTIM_PERIOD: DTIM period for beaconing
 * @NL80211_ATTR_BEACON_HEAD: portion of the beacon before the TIM IE
 * @NL80211_ATTR_BEACON_TAIL: portion of the beacon after the TIM IE
 *
 * @NL80211_ATTR_STA_AID: Association ID for the station (u16)
 * @NL80211_ATTR_STA_FLAGS: flags, nested element with NLA_FLAG attributes of
 *	&enum nl80211_sta_flags (deprecated, use %NL80211_ATTR_STA_FLAGS2)
 * @NL80211_ATTR_STA_LISTEN_INTERVAL: listen interval as defined by
 *	IEEE 802.11 7.3.1.6 (u16).
 * @NL80211_ATTR_STA_SUPPORTED_RATES: supported rates, array of supported
 *	rates as defined by IEEE 802.11 7.3.2.2 but without the length
 *	restriction (at most %NL80211_MAX_SUPP_RATES).
 * @NL80211_ATTR_STA_VLAN: interface index of VLAN interface to move station
 *	to, or the AP interface the station was originally added to to.
 * @NL80211_ATTR_STA_INFO: information about a station, part of station info
 *	given for %NL80211_CMD_GET_STATION, nested attribute containing
 *	info as possible, see &enum nl80211_sta_info.
 *
 * @NL80211_ATTR_WIPHY_BANDS: Information about an operating bands,
 *	consisting of a nested array.
 *
 * @NL80211_ATTR_MESH_ID: mesh id (1-32 bytes).
 * @NL80211_ATTR_STA_PLINK_ACTION: action to perform on the mesh peer link.
 * @NL80211_ATTR_MPATH_NEXT_HOP: MAC address of the next hop for a mesh path.
 * @NL80211_ATTR_MPATH_INFO: information about a mesh_path, part of mesh path
 * 	info given for %NL80211_CMD_GET_MPATH, nested attribute described at
 *	&enum nl80211_mpath_info.
 *
 * @NL80211_ATTR_MNTR_FLAGS: flags, nested element with NLA_FLAG attributes of
 *      &enum nl80211_mntr_flags.
 *
 * @NL80211_ATTR_REG_ALPHA2: an ISO-3166-alpha2 country code for which the
 * 	current regulatory domain should be set to or is already set to.
 * 	For example, 'CR', for Costa Rica. This attribute is used by the kernel
 * 	to query the CRDA to retrieve one regulatory domain. This attribute can
 * 	also be used by userspace to query the kernel for the currently set
 * 	regulatory domain. We chose an alpha2 as that is also used by the
 * 	IEEE-802.11d country information element to identify a country.
 * 	Users can also simply ask the wireless core to set regulatory domain
 * 	to a specific alpha2.
 * @NL80211_ATTR_REG_RULES: a nested array of regulatory domain regulatory
 *	rules.
 *
 * @NL80211_ATTR_BSS_CTS_PROT: whether CTS protection is enabled (u8, 0 or 1)
 * @NL80211_ATTR_BSS_SHORT_PREAMBLE: whether short preamble is enabled
 *	(u8, 0 or 1)
 * @NL80211_ATTR_BSS_SHORT_SLOT_TIME: whether short slot time enabled
 *	(u8, 0 or 1)
 * @NL80211_ATTR_BSS_BASIC_RATES: basic rates, array of basic
 *	rates in format defined by IEEE 802.11 7.3.2.2 but without the length
 *	restriction (at most %NL80211_MAX_SUPP_RATES).
 *
 * @NL80211_ATTR_HT_CAPABILITY: HT Capability information element (from
 *	association request when used with NL80211_CMD_NEW_STATION)
 *
 * @NL80211_ATTR_SUPPORTED_IFTYPES: nested attribute containing all
 *	supported interface types, each a flag attribute with the number
 *	of the interface mode.
 *
 * @NL80211_ATTR_MGMT_SUBTYPE: Management frame subtype for
 *	%NL80211_CMD_SET_MGMT_EXTRA_IE.
 *
 * @NL80211_ATTR_IE: Information element(s) data (used, e.g., with
 *	%NL80211_CMD_SET_MGMT_EXTRA_IE).
 *
 * @NL80211_ATTR_MAX_NUM_SCAN_SSIDS: number of SSIDs you can scan with
 *	a single scan request, a wiphy attribute.
 * @NL80211_ATTR_MAX_NUM_SCHED_SCAN_SSIDS: number of SSIDs you can
 *	scan with a single scheduled scan request, a wiphy attribute.
 * @NL80211_ATTR_MAX_SCAN_IE_LEN: maximum length of information elements
 *	that can be added to a scan request
 * @NL80211_ATTR_MAX_SCHED_SCAN_IE_LEN: maximum length of information
 *	elements that can be added to a scheduled scan request
 * @NL80211_ATTR_MAX_MATCH_SETS: maximum number of sets that can be
 *	used with @NL80211_ATTR_SCHED_SCAN_MATCH, a wiphy attribute.
 *
 * @NL80211_ATTR_SCAN_FREQUENCIES: nested attribute with frequencies (in MHz)
 * @NL80211_ATTR_SCAN_SSIDS: nested attribute with SSIDs, leave out for passive
 *	scanning and include a zero-length SSID (wildcard) for wildcard scan
 * @NL80211_ATTR_BSS: scan result BSS
 *
 * @NL80211_ATTR_REG_INITIATOR: indicates who requested the regulatory domain
 * 	currently in effect. This could be any of the %NL80211_REGDOM_SET_BY_*
 * @NL80211_ATTR_REG_TYPE: indicates the type of the regulatory domain currently
 * 	set. This can be one of the nl80211_reg_type (%NL80211_REGDOM_TYPE_*)
 *
 * @NL80211_ATTR_SUPPORTED_COMMANDS: wiphy attribute that specifies
 *	an array of command numbers (i.e. a mapping index to command number)
 *	that the driver for the given wiphy supports.
 *
 * @NL80211_ATTR_FRAME: frame data (binary attribute), including frame header
 *	and body, but not FCS; used, e.g., with NL80211_CMD_AUTHENTICATE and
 *	NL80211_CMD_ASSOCIATE events
 * @NL80211_ATTR_SSID: SSID (binary attribute, 0..32 octets)
 * @NL80211_ATTR_AUTH_TYPE: AuthenticationType, see &enum nl80211_auth_type,
 *	represented as a u32
 * @NL80211_ATTR_REASON_CODE: ReasonCode for %NL80211_CMD_DEAUTHENTICATE and
 *	%NL80211_CMD_DISASSOCIATE, u16
 *
 * @NL80211_ATTR_KEY_TYPE: Key Type, see &enum nl80211_key_type, represented as
 *	a u32
 *
 * @NL80211_ATTR_FREQ_BEFORE: A channel which has suffered a regulatory change
 * 	due to considerations from a beacon hint. This attribute reflects
 * 	the state of the channel _before_ the beacon hint processing. This
 * 	attributes consists of a nested attribute containing
 * 	NL80211_FREQUENCY_ATTR_*
 * @NL80211_ATTR_FREQ_AFTER: A channel which has suffered a regulatory change
 * 	due to considerations from a beacon hint. This attribute reflects
 * 	the state of the channel _after_ the beacon hint processing. This
 * 	attributes consists of a nested attribute containing
 * 	NL80211_FREQUENCY_ATTR_*
 *
 * @NL80211_ATTR_CIPHER_SUITES: a set of u32 values indicating the supported
 *	cipher suites
 *
 * @NL80211_ATTR_FREQ_FIXED: a flag indicating the IBSS should not try to look
 *	for other networks on different channels
 *
 * @NL80211_ATTR_TIMED_OUT: a flag indicating than an operation timed out; this
 *	is used, e.g., with %NL80211_CMD_AUTHENTICATE event
 *
 * @NL80211_ATTR_USE_MFP: Whether management frame protection (IEEE 802.11w) is
 *	used for the association (&enum nl80211_mfp, represented as a u32);
 *	this attribute can be used
 *	with %NL80211_CMD_ASSOCIATE request
 *
 * @NL80211_ATTR_STA_FLAGS2: Attribute containing a
 *	&struct nl80211_sta_flag_update.
 *
 * @NL80211_ATTR_CONTROL_PORT: A flag indicating whether user space controls
 *	IEEE 802.1X port, i.e., sets/clears %NL80211_STA_FLAG_AUTHORIZED, in
 *	station mode. If the flag is included in %NL80211_CMD_ASSOCIATE
 *	request, the driver will assume that the port is unauthorized until
 *	authorized by user space. Otherwise, port is marked authorized by
 *	default in station mode.
 * @NL80211_ATTR_CONTROL_PORT_ETHERTYPE: A 16-bit value indicating the
 *	ethertype that will be used for key negotiation. It can be
 *	specified with the associate and connect commands. If it is not
 *	specified, the value defaults to 0x888E (PAE, 802.1X). This
 *	attribute is also used as a flag in the wiphy information to
 *	indicate that protocols other than PAE are supported.
 * @NL80211_ATTR_CONTROL_PORT_NO_ENCRYPT: When included along with
 *	%NL80211_ATTR_CONTROL_PORT_ETHERTYPE, indicates that the custom
 *	ethertype frames used for key negotiation must not be encrypted.
 *
 * @NL80211_ATTR_TESTDATA: Testmode data blob, passed through to the driver.
 *	We recommend using nested, driver-specific attributes within this.
 *
 * @NL80211_ATTR_DISCONNECTED_BY_AP: A flag indicating that the DISCONNECT
 *	event was due to the AP disconnecting the station, and not due to
 *	a local disconnect request.
 * @NL80211_ATTR_STATUS_CODE: StatusCode for the %NL80211_CMD_CONNECT
 *	event (u16)
 * @NL80211_ATTR_PRIVACY: Flag attribute, used with connect(), indicating
 *	that protected APs should be used. This is also used with NEW_BEACON to
 *	indicate that the BSS is to use protection.
 *
 * @NL80211_ATTR_CIPHERS_PAIRWISE: Used with CONNECT, ASSOCIATE, and NEW_BEACON
 *	to indicate which unicast key ciphers will be used with the connection
 *	(an array of u32).
 * @NL80211_ATTR_CIPHER_GROUP: Used with CONNECT, ASSOCIATE, and NEW_BEACON to
 *	indicate which group key cipher will be used with the connection (a
 *	u32).
 * @NL80211_ATTR_WPA_VERSIONS: Used with CONNECT, ASSOCIATE, and NEW_BEACON to
 *	indicate which WPA version(s) the AP we want to associate with is using
 *	(a u32 with flags from &enum nl80211_wpa_versions).
 * @NL80211_ATTR_AKM_SUITES: Used with CONNECT, ASSOCIATE, and NEW_BEACON to
 *	indicate which key management algorithm(s) to use (an array of u32).
 *
 * @NL80211_ATTR_REQ_IE: (Re)association request information elements as
 *	sent out by the card, for ROAM and successful CONNECT events.
 * @NL80211_ATTR_RESP_IE: (Re)association response information elements as
 *	sent by peer, for ROAM and successful CONNECT events.
 *
 * @NL80211_ATTR_PREV_BSSID: previous BSSID, to be used by in ASSOCIATE
 *	commands to specify using a reassociate frame
 *
 * @NL80211_ATTR_KEY: key information in a nested attribute with
 *	%NL80211_KEY_* sub-attributes
 * @NL80211_ATTR_KEYS: array of keys for static WEP keys for connect()
 *	and join_ibss(), key information is in a nested attribute each
 *	with %NL80211_KEY_* sub-attributes
 *
 * @NL80211_ATTR_PID: Process ID of a network namespace.
 *
 * @NL80211_ATTR_GENERATION: Used to indicate consistent snapshots for
 *	dumps. This number increases whenever the object list being
 *	dumped changes, and as such userspace can verify that it has
 *	obtained a complete and consistent snapshot by verifying that
 *	all dump messages contain the same generation number. If it
 *	changed then the list changed and the dump should be repeated
 *	completely from scratch.
 *
 * @NL80211_ATTR_4ADDR: Use 4-address frames on a virtual interface
 *
 * @NL80211_ATTR_SURVEY_INFO: survey information about a channel, part of
 *      the survey response for %NL80211_CMD_GET_SURVEY, nested attribute
 *      containing info as possible, see &enum survey_info.
 *
 * @NL80211_ATTR_PMKID: PMK material for PMKSA caching.
 * @NL80211_ATTR_MAX_NUM_PMKIDS: maximum number of PMKIDs a firmware can
 *	cache, a wiphy attribute.
 *
 * @NL80211_ATTR_DURATION: Duration of an operation in milliseconds, u32.
 * @NL80211_ATTR_MAX_REMAIN_ON_CHANNEL_DURATION: Device attribute that
 *	specifies the maximum duration that can be requested with the
 *	remain-on-channel operation, in milliseconds, u32.
 *
 * @NL80211_ATTR_COOKIE: Generic 64-bit cookie to identify objects.
 *
 * @NL80211_ATTR_TX_RATES: Nested set of attributes
 *	(enum nl80211_tx_rate_attributes) describing TX rates per band. The
 *	enum nl80211_band value is used as the index (nla_type() of the nested
 *	data. If a band is not included, it will be configured to allow all
 *	rates based on negotiated supported rates information. This attribute
 *	is used with %NL80211_CMD_SET_TX_BITRATE_MASK.
 *
 * @NL80211_ATTR_FRAME_MATCH: A binary attribute which typically must contain
 *	at least one byte, currently used with @NL80211_CMD_REGISTER_FRAME.
 * @NL80211_ATTR_FRAME_TYPE: A u16 indicating the frame type/subtype for the
 *	@NL80211_CMD_REGISTER_FRAME command.
 * @NL80211_ATTR_TX_FRAME_TYPES: wiphy capability attribute, which is a
 *	nested attribute of %NL80211_ATTR_FRAME_TYPE attributes, containing
 *	information about which frame types can be transmitted with
 *	%NL80211_CMD_FRAME.
 * @NL80211_ATTR_RX_FRAME_TYPES: wiphy capability attribute, which is a
 *	nested attribute of %NL80211_ATTR_FRAME_TYPE attributes, containing
 *	information about which frame types can be registered for RX.
 *
 * @NL80211_ATTR_ACK: Flag attribute indicating that the frame was
 *	acknowledged by the recipient.
 *
 * @NL80211_ATTR_CQM: connection quality monitor configuration in a
 *	nested attribute with %NL80211_ATTR_CQM_* sub-attributes.
 *
 * @NL80211_ATTR_LOCAL_STATE_CHANGE: Flag attribute to indicate that a command
 *	is requesting a local authentication/association state change without
 *	invoking actual management frame exchange. This can be used with
 *	NL80211_CMD_AUTHENTICATE, NL80211_CMD_DEAUTHENTICATE,
 *	NL80211_CMD_DISASSOCIATE.
 *
 * @NL80211_ATTR_AP_ISOLATE: (AP mode) Do not forward traffic between stations
 *	connected to this BSS.
 *
 * @NL80211_ATTR_WIPHY_TX_POWER_SETTING: Transmit power setting type. See
 *      &enum nl80211_tx_power_setting for possible values.
 * @NL80211_ATTR_WIPHY_TX_POWER_LEVEL: Transmit power level in signed mBm units.
 *      This is used in association with @NL80211_ATTR_WIPHY_TX_POWER_SETTING
 *      for non-automatic settings.
 *
 * @NL80211_ATTR_SUPPORT_IBSS_RSN: The device supports IBSS RSN, which mostly
 *	means support for per-station GTKs.
 *
 * @NL80211_ATTR_WIPHY_ANTENNA_TX: Bitmap of allowed antennas for transmitting.
 *	This can be used to mask out antennas which are not attached or should
 *	not be used for transmitting. If an antenna is not selected in this
 *	bitmap the hardware is not allowed to transmit on this antenna.
 *
 *	Each bit represents one antenna, starting with antenna 1 at the first
 *	bit. Depending on which antennas are selected in the bitmap, 802.11n
 *	drivers can derive which chainmasks to use (if all antennas belonging to
 *	a particular chain are disabled this chain should be disabled) and if
 *	a chain has diversity antennas wether diversity should be used or not.
 *	HT capabilities (STBC, TX Beamforming, Antenna selection) can be
 *	derived from the available chains after applying the antenna mask.
 *	Non-802.11n drivers can derive wether to use diversity or not.
 *	Drivers may reject configurations or RX/TX mask combinations they cannot
 *	support by returning -EINVAL.
 *
 * @NL80211_ATTR_WIPHY_ANTENNA_RX: Bitmap of allowed antennas for receiving.
 *	This can be used to mask out antennas which are not attached or should
 *	not be used for receiving. If an antenna is not selected in this bitmap
 *	the hardware should not be configured to receive on this antenna.
 *	For a more detailed description see @NL80211_ATTR_WIPHY_ANTENNA_TX.
 *
 * @NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX: Bitmap of antennas which are available
 *	for configuration as TX antennas via the above parameters.
 *
 * @NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX: Bitmap of antennas which are available
 *	for configuration as RX antennas via the above parameters.
 *
 * @NL80211_ATTR_MCAST_RATE: Multicast tx rate (in 100 kbps) for IBSS
 *
 * @NL80211_ATTR_OFFCHANNEL_TX_OK: For management frame TX, the frame may be
 *	transmitted on another channel when the channel given doesn't match
 *	the current channel. If the current channel doesn't match and this
 *	flag isn't set, the frame will be rejected. This is also used as an
 *	nl80211 capability flag.
 *
 * @NL80211_ATTR_BSS_HTOPMODE: HT operation mode (u16)
 *
 * @NL80211_ATTR_KEY_DEFAULT_TYPES: A nested attribute containing flags
 *	attributes, specifying what a key should be set as default as.
 *	See &enum nl80211_key_default_types.
 *
 * @NL80211_ATTR_MESH_SETUP: Optional mesh setup parameters.  These cannot be
 *	changed once the mesh is active.
 * @NL80211_ATTR_MESH_CONFIG: Mesh configuration parameters, a nested attribute
 *	containing attributes from &enum nl80211_meshconf_params.
 * @NL80211_ATTR_SUPPORT_MESH_AUTH: Currently, this means the underlying driver
 *	allows auth frames in a mesh to be passed to userspace for processing via
 *	the @NL80211_MESH_SETUP_USERSPACE_AUTH flag.
 * @NL80211_ATTR_STA_PLINK_STATE: The state of a mesh peer link as
 *	defined in &enum nl80211_plink_state. Used when userspace is
 *	driving the peer link management state machine.
 *	@NL80211_MESH_SETUP_USERSPACE_AMPE must be enabled.
 *
 * @NL80211_ATTR_WOWLAN_TRIGGERS_SUPPORTED: indicates, as part of the wiphy
 *	capabilities, the supported WoWLAN triggers
 * @NL80211_ATTR_WOWLAN_TRIGGERS: used by %NL80211_CMD_SET_WOWLAN to
 *	indicate which WoW triggers should be enabled. This is also
 *	used by %NL80211_CMD_GET_WOWLAN to get the currently enabled WoWLAN
 *	triggers.

 * @NL80211_ATTR_SCHED_SCAN_INTERVAL: Interval between scheduled scan
 *	cycles, in msecs.

 * @NL80211_ATTR_SCHED_SCAN_MATCH: Nested attribute with one or more
 *	sets of attributes to match during scheduled scans.  Only BSSs
 *	that match any of the sets will be reported.  These are
 *	pass-thru filter rules.
 *	For a match to succeed, the BSS must match all attributes of a
 *	set.  Since not every hardware supports matching all types of
 *	attributes, there is no guarantee that the reported BSSs are
 *	fully complying with the match sets and userspace needs to be
 *	able to ignore them by itself.
 *	Thus, the implementation is somewhat hardware-dependent, but
 *	this is only an optimization and the userspace application
 *	needs to handle all the non-filtered results anyway.
 *	If the match attributes don't make sense when combined with
 *	the values passed in @NL80211_ATTR_SCAN_SSIDS (eg. if an SSID
 *	is included in the probe request, but the match attributes
 *	will never let it go through), -EINVAL may be returned.
 *	If ommited, no filtering is done.
 *
 * @NL80211_ATTR_INTERFACE_COMBINATIONS: Nested attribute listing the supported
 *	interface combinations. In each nested item, it contains attributes
 *	defined in &enum nl80211_if_combination_attrs.
 * @NL80211_ATTR_SOFTWARE_IFTYPES: Nested attribute (just like
 *	%NL80211_ATTR_SUPPORTED_IFTYPES) containing the interface types that
 *	are managed in software: interfaces of these types aren't subject to
 *	any restrictions in their number or combinations.
 *
 * @%NL80211_ATTR_REKEY_DATA: nested attribute containing the information
 *	necessary for GTK rekeying in the device, see &enum nl80211_rekey_data.
 *
 * @NL80211_ATTR_SCAN_SUPP_RATES: rates per to be advertised as supported in scan,
 *	nested array attribute containing an entry for each band, with the entry
 *	being a list of supported rates as defined by IEEE 802.11 7.3.2.2 but
 *	without the length restriction (at most %NL80211_MAX_SUPP_RATES).
 *
 * @NL80211_ATTR_HIDDEN_SSID: indicates whether SSID is to be hidden from Beacon
 *	and Probe Response (when response to wildcard Probe Request); see
 *	&enum nl80211_hidden_ssid, represented as a u32
 *
 * @NL80211_ATTR_IE_PROBE_RESP: Information element(s) for Probe Response frame.
 *	This is used with %NL80211_CMD_NEW_BEACON and %NL80211_CMD_SET_BEACON to
 *	provide extra IEs (e.g., WPS/P2P IE) into Probe Response frames when the
 *	driver (or firmware) replies to Probe Request frames.
 * @NL80211_ATTR_IE_ASSOC_RESP: Information element(s) for (Re)Association
 *	Response frames. This is used with %NL80211_CMD_NEW_BEACON and
 *	%NL80211_CMD_SET_BEACON to provide extra IEs (e.g., WPS/P2P IE) into
 *	(Re)Association Response frames when the driver (or firmware) replies to
 *	(Re)Association Request frames.
 *
 * @NL80211_ATTR_STA_WME: Nested attribute containing the wme configuration
 *	of the station, see &enum nl80211_sta_wme_attr.
 * @NL80211_ATTR_SUPPORT_AP_UAPSD: the device supports uapsd when working
 *	as AP.
 *
 * @NL80211_ATTR_ROAM_SUPPORT: Indicates whether the firmware is capable of
 *	roaming to another AP in the same ESS if the signal lever is low.
 *
 * @NL80211_ATTR_PMKSA_CANDIDATE: Nested attribute containing the PMKSA caching
 *	candidate information, see &enum nl80211_pmksa_candidate_attr.
 *
 * @NL80211_ATTR_TX_NO_CCK_RATE: Indicates whether to use CCK rate or not
 *	for management frames transmission. In order to avoid p2p probe/action
 *	frames are being transmitted at CCK rate in 2GHz band, the user space
 *	applications use this attribute.
 *	This attribute is used with %NL80211_CMD_TRIGGER_SCAN and
 *	%NL80211_CMD_FRAME commands.
 *
 * @NL80211_ATTR_TDLS_ACTION: Low level TDLS action code (e.g. link setup
 *	request, link setup confirm, link teardown, etc.). Values are
 *	described in the TDLS (802.11z) specification.
 * @NL80211_ATTR_TDLS_DIALOG_TOKEN: Non-zero token for uniquely identifying a
 *	TDLS conversation between two devices.
 * @NL80211_ATTR_TDLS_OPERATION: High level TDLS operation; see
 *	&enum nl80211_tdls_operation, represented as a u8.
 * @NL80211_ATTR_TDLS_SUPPORT: A flag indicating the device can operate
 *	as a TDLS peer sta.
 * @NL80211_ATTR_TDLS_EXTERNAL_SETUP: The TDLS discovery/setup and teardown
 *	procedures should be performed by sending TDLS packets via
 *	%NL80211_CMD_TDLS_MGMT. Otherwise %NL80211_CMD_TDLS_OPER should be
 *	used for asking the driver to perform a TDLS operation.
 *
 * @NL80211_ATTR_DEVICE_AP_SME: This u32 attribute may be listed for devices
 *	that have AP support to indicate that they have the AP SME integrated
 *	with support for the features listed in this attribute, see
 *	&enum nl80211_ap_sme_features.
 *
 * @NL80211_ATTR_DONT_WAIT_FOR_ACK: Used with %NL80211_CMD_FRAME, this tells
 *	the driver to not wait for an acknowledgement. Note that due to this,
 *	it will also not give a status callback nor return a cookie. This is
 *	mostly useful for probe responses to save airtime.
 *
 * @NL80211_ATTR_FEATURE_FLAGS: This u32 attribute contains flags from
 *	&enum nl80211_feature_flags and is advertised in wiphy information.
 * @NL80211_ATTR_PROBE_RESP_OFFLOAD: Indicates that the HW responds to probe
 *
 *	requests while operating in AP-mode.
 *	This attribute holds a bitmap of the supported protocols for
 *	offloading (see &enum nl80211_probe_resp_offload_support_attr).
 *
 * @NL80211_ATTR_PROBE_RESP: Probe Response template data. Contains the entire
 *	probe-response frame. The DA field in the 802.11 header is zero-ed out,
 *	to be filled by the FW.
 * @NL80211_ATTR_DISABLE_HT:  Force HT capable interfaces to disable
 *      this feature.  Currently, only supported in mac80211 drivers.
 * @NL80211_ATTR_HT_CAPABILITY_MASK: Specify which bits of the
 *      ATTR_HT_CAPABILITY to which attention should be paid.
 *      Currently, only mac80211 NICs support this feature.
 *      The values that may be configured are:
 *       MCS rates, MAX-AMSDU, HT-20-40 and HT_CAP_SGI_40
 *       AMPDU density and AMPDU factor.
 *      All values are treated as suggestions and may be ignored
 *      by the driver as required.  The actual values may be seen in
 *      the station debugfs ht_caps file.
 *
 * @NL80211_ATTR_DFS_REGION: region for regulatory rules which this country
 *    abides to when initiating radiation on DFS channels. A country maps
 *    to one DFS region.
 *
 * @NL80211_ATTR_NOACK_MAP: This u16 bitmap contains the No Ack Policy of
 *      up to 16 TIDs.
 *
 * @NL80211_ATTR_MAX: highest attribute number currently defined
 * @__NL80211_ATTR_AFTER_LAST: internal use
 */
enum nl80211_attrs {
/* don't change the order or add anything between, this is ABI! */
	NL80211_ATTR_UNSPEC,

	NL80211_ATTR_WIPHY,
	NL80211_ATTR_WIPHY_NAME,

	NL80211_ATTR_IFINDEX,
	NL80211_ATTR_IFNAME,
	NL80211_ATTR_IFTYPE,

	NL80211_ATTR_MAC,

	NL80211_ATTR_KEY_DATA,
	NL80211_ATTR_KEY_IDX,
	NL80211_ATTR_KEY_CIPHER,
	NL80211_ATTR_KEY_SEQ,
	NL80211_ATTR_KEY_DEFAULT,

	NL80211_ATTR_BEACON_INTERVAL,
	NL80211_ATTR_DTIM_PERIOD,
	NL80211_ATTR_BEACON_HEAD,
	NL80211_ATTR_BEACON_TAIL,

	NL80211_ATTR_STA_AID,
	NL80211_ATTR_STA_FLAGS,
	NL80211_ATTR_STA_LISTEN_INTERVAL,
	NL80211_ATTR_STA_SUPPORTED_RATES,
	NL80211_ATTR_STA_VLAN,
	NL80211_ATTR_STA_INFO,

	NL80211_ATTR_WIPHY_BANDS,

	NL80211_ATTR_MNTR_FLAGS,

	NL80211_ATTR_MESH_ID,
	NL80211_ATTR_STA_PLINK_ACTION,
	NL80211_ATTR_MPATH_NEXT_HOP,
	NL80211_ATTR_MPATH_INFO,

	NL80211_ATTR_BSS_CTS_PROT,
	NL80211_ATTR_BSS_SHORT_PREAMBLE,
	NL80211_ATTR_BSS_SHORT_SLOT_TIME,

	NL80211_ATTR_HT_CAPABILITY,

	NL80211_ATTR_SUPPORTED_IFTYPES,

	NL80211_ATTR_REG_ALPHA2,
	NL80211_ATTR_REG_RULES,

	NL80211_ATTR_MESH_CONFIG,

	NL80211_ATTR_BSS_BASIC_RATES,

	NL80211_ATTR_WIPHY_TXQ_PARAMS,
	NL80211_ATTR_WIPHY_FREQ,
	NL80211_ATTR_WIPHY_CHANNEL_TYPE,

	NL80211_ATTR_KEY_DEFAULT_MGMT,

	NL80211_ATTR_MGMT_SUBTYPE,
	NL80211_ATTR_IE,

	NL80211_ATTR_MAX_NUM_SCAN_SSIDS,

	NL80211_ATTR_SCAN_FREQUENCIES,
	NL80211_ATTR_SCAN_SSIDS,
	NL80211_ATTR_GENERATION, /* replaces old SCAN_GENERATION */
	NL80211_ATTR_BSS,

	NL80211_ATTR_REG_INITIATOR,
	NL80211_ATTR_REG_TYPE,

	NL80211_ATTR_SUPPORTED_COMMANDS,

	NL80211_ATTR_FRAME,
	NL80211_ATTR_SSID,
	NL80211_ATTR_AUTH_TYPE,
	NL80211_ATTR_REASON_CODE,

	NL80211_ATTR_KEY_TYPE,

	NL80211_ATTR_MAX_SCAN_IE_LEN,
	NL80211_ATTR_CIPHER_SUITES,

	NL80211_ATTR_FREQ_BEFORE,
	NL80211_ATTR_FREQ_AFTER,

	NL80211_ATTR_FREQ_FIXED,


	NL80211_ATTR_WIPHY_RETRY_SHORT,
	NL80211_ATTR_WIPHY_RETRY_LONG,
	NL80211_ATTR_WIPHY_FRAG_THRESHOLD,
	NL80211_ATTR_WIPHY_RTS_THRESHOLD,

	NL80211_ATTR_TIMED_OUT,

	NL80211_ATTR_USE_MFP,

	NL80211_ATTR_STA_FLAGS2,

	NL80211_ATTR_CONTROL_PORT,

	NL80211_ATTR_TESTDATA,

	NL80211_ATTR_PRIVACY,

	NL80211_ATTR_DISCONNECTED_BY_AP,
	NL80211_ATTR_STATUS_CODE,

	NL80211_ATTR_CIPHER_SUITES_PAIRWISE,
	NL80211_ATTR_CIPHER_SUITE_GROUP,
	NL80211_ATTR_WPA_VERSIONS,
	NL80211_ATTR_AKM_SUITES,

	NL80211_ATTR_REQ_IE,
	NL80211_ATTR_RESP_IE,

	NL80211_ATTR_PREV_BSSID,

	NL80211_ATTR_KEY,
	NL80211_ATTR_KEYS,

	NL80211_ATTR_PID,

	NL80211_ATTR_4ADDR,

	NL80211_ATTR_SURVEY_INFO,

	NL80211_ATTR_PMKID,
	NL80211_ATTR_MAX_NUM_PMKIDS,

	NL80211_ATTR_DURATION,

	NL80211_ATTR_COOKIE,

	NL80211_ATTR_WIPHY_COVERAGE_CLASS,

	NL80211_ATTR_TX_RATES,

	NL80211_ATTR_FRAME_MATCH,

	NL80211_ATTR_ACK,

	NL80211_ATTR_PS_STATE,

	NL80211_ATTR_CQM,

	NL80211_ATTR_LOCAL_STATE_CHANGE,

	NL80211_ATTR_AP_ISOLATE,

	NL80211_ATTR_WIPHY_TX_POWER_SETTING,
	NL80211_ATTR_WIPHY_TX_POWER_LEVEL,

	NL80211_ATTR_TX_FRAME_TYPES,
	NL80211_ATTR_RX_FRAME_TYPES,
	NL80211_ATTR_FRAME_TYPE,

	NL80211_ATTR_CONTROL_PORT_ETHERTYPE,
	NL80211_ATTR_CONTROL_PORT_NO_ENCRYPT,

	NL80211_ATTR_SUPPORT_IBSS_RSN,

	NL80211_ATTR_WIPHY_ANTENNA_TX,
	NL80211_ATTR_WIPHY_ANTENNA_RX,

	NL80211_ATTR_MCAST_RATE,

	NL80211_ATTR_OFFCHANNEL_TX_OK,

	NL80211_ATTR_BSS_HT_OPMODE,

	NL80211_ATTR_KEY_DEFAULT_TYPES,

	NL80211_ATTR_MAX_REMAIN_ON_CHANNEL_DURATION,

	NL80211_ATTR_MESH_SETUP,

	NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX,
	NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX,

	NL80211_ATTR_SUPPORT_MESH_AUTH,
	NL80211_ATTR_STA_PLINK_STATE,

	NL80211_ATTR_WOWLAN_TRIGGERS,
	NL80211_ATTR_WOWLAN_TRIGGERS_SUPPORTED,

	NL80211_ATTR_SCHED_SCAN_INTERVAL,

	NL80211_ATTR_INTERFACE_COMBINATIONS,
	NL80211_ATTR_SOFTWARE_IFTYPES,

	NL80211_ATTR_REKEY_DATA,

	NL80211_ATTR_MAX_NUM_SCHED_SCAN_SSIDS,
	NL80211_ATTR_MAX_SCHED_SCAN_IE_LEN,

	NL80211_ATTR_SCAN_SUPP_RATES,

	NL80211_ATTR_HIDDEN_SSID,

	NL80211_ATTR_IE_PROBE_RESP,
	NL80211_ATTR_IE_ASSOC_RESP,

	NL80211_ATTR_STA_WME,
	NL80211_ATTR_SUPPORT_AP_UAPSD,

	NL80211_ATTR_ROAM_SUPPORT,

	NL80211_ATTR_SCHED_SCAN_MATCH,
	NL80211_ATTR_MAX_MATCH_SETS,

	NL80211_ATTR_PMKSA_CANDIDATE,

	NL80211_ATTR_TX_NO_CCK_RATE,

	NL80211_ATTR_TDLS_ACTION,
	NL80211_ATTR_TDLS_DIALOG_TOKEN,
	NL80211_ATTR_TDLS_OPERATION,
	NL80211_ATTR_TDLS_SUPPORT,
	NL80211_ATTR_TDLS_EXTERNAL_SETUP,

	NL80211_ATTR_DEVICE_AP_SME,

	NL80211_ATTR_DONT_WAIT_FOR_ACK,

	NL80211_ATTR_FEATURE_FLAGS,

	NL80211_ATTR_PROBE_RESP_OFFLOAD,

	NL80211_ATTR_PROBE_RESP,

	NL80211_ATTR_DFS_REGION,

	NL80211_ATTR_DISABLE_HT,
	NL80211_ATTR_HT_CAPABILITY_MASK,

	NL80211_ATTR_NOACK_MAP,

	NL80211_ATTR_INACTIVITY_TIMEOUT,

	NL80211_ATTR_RX_SIGNAL_DBM,

	NL80211_ATTR_BG_SCAN_PERIOD,

	NL80211_ATTR_WDEV,

	NL80211_ATTR_USER_REG_HINT_TYPE,

	NL80211_ATTR_CONN_FAILED_REASON,

	NL80211_ATTR_AUTH_DATA,

	NL80211_ATTR_VHT_CAPABILITY,

	NL80211_ATTR_SCAN_FLAGS,

	NL80211_ATTR_CHANNEL_WIDTH,
	NL80211_ATTR_CENTER_FREQ1,
	NL80211_ATTR_CENTER_FREQ2,

	NL80211_ATTR_P2P_CTWINDOW,
	NL80211_ATTR_P2P_OPPPS,

	NL80211_ATTR_LOCAL_MESH_POWER_MODE,

	NL80211_ATTR_ACL_POLICY,

	NL80211_ATTR_MAC_ADDRS,

	NL80211_ATTR_MAC_ACL_MAX,

	NL80211_ATTR_RADAR_EVENT,

	NL80211_ATTR_EXT_CAPA,
	NL80211_ATTR_EXT_CAPA_MASK,

	NL80211_ATTR_STA_CAPABILITY,
	NL80211_ATTR_STA_EXT_CAPABILITY,

	NL80211_ATTR_PROTOCOL_FEATURES,
	NL80211_ATTR_SPLIT_WIPHY_DUMP,

	NL80211_ATTR_DISABLE_VHT,
	NL80211_ATTR_VHT_CAPABILITY_MASK,

	NL80211_ATTR_MDID,
	NL80211_ATTR_IE_RIC,

	NL80211_ATTR_CRIT_PROT_ID,
	NL80211_ATTR_MAX_CRIT_PROT_DURATION,

	NL80211_ATTR_PEER_AID,

	NL80211_ATTR_COALESCE_RULE,

	NL80211_ATTR_CH_SWITCH_COUNT,
	NL80211_ATTR_CH_SWITCH_BLOCK_TX,
	NL80211_ATTR_CSA_IES,
	NL80211_ATTR_CSA_C_OFF_BEACON,
	NL80211_ATTR_CSA_C_OFF_PRESP,

	NL80211_ATTR_RXMGMT_FLAGS,

	NL80211_ATTR_STA_SUPPORTED_CHANNELS,

	NL80211_ATTR_STA_SUPPORTED_OPER_CLASSES,

	NL80211_ATTR_HANDLE_DFS,

	NL80211_ATTR_SUPPORT_5_MHZ,
	NL80211_ATTR_SUPPORT_10_MHZ,

	NL80211_ATTR_OPMODE_NOTIF,

	NL80211_ATTR_VENDOR_ID,
	NL80211_ATTR_VENDOR_SUBCMD,
	NL80211_ATTR_VENDOR_DATA,
	NL80211_ATTR_VENDOR_EVENTS,

	NL80211_ATTR_QOS_MAP,

	NL80211_ATTR_MAC_HINT,
	NL80211_ATTR_WIPHY_FREQ_HINT,

	NL80211_ATTR_MAX_AP_ASSOC_STA,

	NL80211_ATTR_TDLS_PEER_CAPABILITY,

	NL80211_ATTR_SOCKET_OWNER,

	NL80211_ATTR_CSA_C_OFFSETS_TX,
	NL80211_ATTR_MAX_CSA_COUNTERS,

	NL80211_ATTR_TDLS_INITIATOR,

	NL80211_ATTR_USE_RRM,

	NL80211_ATTR_WIPHY_DYN_ACK,

	NL80211_ATTR_TSID,
	NL80211_ATTR_USER_PRIO,
	NL80211_ATTR_ADMITTED_TIME,

	NL80211_ATTR_SMPS_MODE,

	NL80211_ATTR_OPER_CLASS,

	NL80211_ATTR_MAC_MASK,

	NL80211_ATTR_WIPHY_SELF_MANAGED_REG,

	NL80211_ATTR_EXT_FEATURES,

	NL80211_ATTR_SURVEY_RADIO_STATS,

	NL80211_ATTR_NETNS_FD,

	NL80211_ATTR_SCHED_SCAN_DELAY,

	NL80211_ATTR_REG_INDOOR,

	NL80211_ATTR_MAX_NUM_SCHED_SCAN_PLANS,
	NL80211_ATTR_MAX_SCAN_PLAN_INTERVAL,
	NL80211_ATTR_MAX_SCAN_PLAN_ITERATIONS,
	NL80211_ATTR_SCHED_SCAN_PLANS,

	NL80211_ATTR_PBSS,

	NL80211_ATTR_BSS_SELECT,

	NL80211_ATTR_STA_SUPPORT_P2P_PS,

	NL80211_ATTR_PAD,

	NL80211_ATTR_IFTYPE_EXT_CAPA,

	NL80211_ATTR_MU_MIMO_GROUP_DATA,
	NL80211_ATTR_MU_MIMO_FOLLOW_MAC_ADDR,

	NL80211_ATTR_SCAN_START_TIME_TSF,
	NL80211_ATTR_SCAN_START_TIME_TSF_BSSID,
	NL80211_ATTR_MEASUREMENT_DURATION,
	NL80211_ATTR_MEASUREMENT_DURATION_MANDATORY,

	NL80211_ATTR_MESH_PEER_AID,

	NL80211_ATTR_NAN_MASTER_PREF,
	NL80211_ATTR_BANDS,
	NL80211_ATTR_NAN_FUNC,
	NL80211_ATTR_NAN_MATCH,

	NL80211_ATTR_FILS_KEK,
	NL80211_ATTR_FILS_NONCES,

	NL80211_ATTR_MULTICAST_TO_UNICAST_ENABLED,

	NL80211_ATTR_BSSID,

	NL80211_ATTR_SCHED_SCAN_RELATIVE_RSSI,
	NL80211_ATTR_SCHED_SCAN_RSSI_ADJUST,

	NL80211_ATTR_TIMEOUT_REASON,

	NL80211_ATTR_FILS_ERP_USERNAME,
	NL80211_ATTR_FILS_ERP_REALM,
	NL80211_ATTR_FILS_ERP_NEXT_SEQ_NUM,
	NL80211_ATTR_FILS_ERP_RRK,
	NL80211_ATTR_FILS_CACHE_ID,

	NL80211_ATTR_PMK,

	NL80211_ATTR_SCHED_SCAN_MULTI,
	NL80211_ATTR_SCHED_SCAN_MAX_REQS,

	NL80211_ATTR_WANT_1X_4WAY_HS,
	NL80211_ATTR_PMKR0_NAME,
	NL80211_ATTR_PORT_AUTHORIZED,

	NL80211_ATTR_EXTERNAL_AUTH_ACTION,
	NL80211_ATTR_EXTERNAL_AUTH_SUPPORT,

	NL80211_ATTR_NSS,
	NL80211_ATTR_ACK_SIGNAL,

	NL80211_ATTR_CONTROL_PORT_OVER_NL80211,

	NL80211_ATTR_TXQ_STATS,
	NL80211_ATTR_TXQ_LIMIT,
	NL80211_ATTR_TXQ_MEMORY_LIMIT,
	NL80211_ATTR_TXQ_QUANTUM,
	NL80211_ATTR_HE_CAPABILITY,


	NL80211_ATTR_FTM_RESPONDER,

	NL80211_ATTR_FTM_RESPONDER_STATS,

	NL80211_ATTR_TIMEOUT,

	NL80211_ATTR_PEER_MEASUREMENTS,

	NL80211_ATTR_AIRTIME_WEIGHT,
	NL80211_ATTR_STA_TX_POWER_SETTING,
	NL80211_ATTR_STA_TX_POWER,
	NL80211_ATTR_SAE_PASSWORD,

	NL80211_ATTR_TWT_RESPONDER,

	NL80211_ATTR_HE_OBSS_PD,

	NL80211_ATTR_WIPHY_EDMG_CHANNELS,
	NL80211_ATTR_WIPHY_EDMG_BW_CONFIG,

	NL80211_ATTR_VLAN_ID,
	NL80211_ATTR_SAE_PWE = 294,

	/* add attributes here, update the policy in nl80211.c */

	__NL80211_ATTR_AFTER_LAST,
	NL80211_ATTR_MAX = __NL80211_ATTR_AFTER_LAST - 1
};

/* source-level API compatibility */
#define NL80211_ATTR_SCAN_GENERATION NL80211_ATTR_GENERATION
#define	NL80211_ATTR_MESH_PARAMS NL80211_ATTR_MESH_CONFIG

/*
 * Allow user space programs to use #ifdef on new attributes by defining them
 * here
 */
#define NL80211_CMD_CONNECT NL80211_CMD_CONNECT
#define NL80211_ATTR_HT_CAPABILITY NL80211_ATTR_HT_CAPABILITY
#define NL80211_ATTR_BSS_BASIC_RATES NL80211_ATTR_BSS_BASIC_RATES
#define NL80211_ATTR_WIPHY_TXQ_PARAMS NL80211_ATTR_WIPHY_TXQ_PARAMS
#define NL80211_ATTR_WIPHY_FREQ NL80211_ATTR_WIPHY_FREQ
#define NL80211_ATTR_WIPHY_CHANNEL_TYPE NL80211_ATTR_WIPHY_CHANNEL_TYPE
#define NL80211_ATTR_MGMT_SUBTYPE NL80211_ATTR_MGMT_SUBTYPE
#define NL80211_ATTR_IE NL80211_ATTR_IE
#define NL80211_ATTR_REG_INITIATOR NL80211_ATTR_REG_INITIATOR
#define NL80211_ATTR_REG_TYPE NL80211_ATTR_REG_TYPE
#define NL80211_ATTR_FRAME NL80211_ATTR_FRAME
#define NL80211_ATTR_SSID NL80211_ATTR_SSID
#define NL80211_ATTR_AUTH_TYPE NL80211_ATTR_AUTH_TYPE
#define NL80211_ATTR_REASON_CODE NL80211_ATTR_REASON_CODE
#define NL80211_ATTR_CIPHER_SUITES_PAIRWISE NL80211_ATTR_CIPHER_SUITES_PAIRWISE
#define NL80211_ATTR_CIPHER_SUITE_GROUP NL80211_ATTR_CIPHER_SUITE_GROUP
#define NL80211_ATTR_WPA_VERSIONS NL80211_ATTR_WPA_VERSIONS
#define NL80211_ATTR_AKM_SUITES NL80211_ATTR_AKM_SUITES
#define NL80211_ATTR_KEY NL80211_ATTR_KEY
#define NL80211_ATTR_KEYS NL80211_ATTR_KEYS
#define NL80211_ATTR_FEATURE_FLAGS NL80211_ATTR_FEATURE_FLAGS

#define NL80211_MAX_SUPP_RATES			32
#define NL80211_MAX_SUPP_HT_RATES		77
#define NL80211_MAX_SUPP_REG_RULES		32
#define NL80211_TKIP_DATA_OFFSET_ENCR_KEY	0
#define NL80211_TKIP_DATA_OFFSET_TX_MIC_KEY	16
#define NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY	24
#define NL80211_HT_CAPABILITY_LEN		26

#define NL80211_MAX_NR_CIPHER_SUITES		5
#define NL80211_MAX_NR_AKM_SUITES		2

/**
 * enum nl80211_iftype - (virtual) interface types
 *
 * @NL80211_IFTYPE_UNSPECIFIED: unspecified type, driver decides
 * @NL80211_IFTYPE_ADHOC: independent BSS member
 * @NL80211_IFTYPE_STATION: managed BSS member
 * @NL80211_IFTYPE_AP: access point
 * @NL80211_IFTYPE_AP_VLAN: VLAN interface for access points; VLAN interfaces
 *	are a bit special in that they must always be tied to a pre-existing
 *	AP type interface.
 * @NL80211_IFTYPE_WDS: wireless distribution interface
 * @NL80211_IFTYPE_MONITOR: monitor interface receiving all frames
 * @NL80211_IFTYPE_MESH_POINT: mesh point
 * @NL80211_IFTYPE_P2P_CLIENT: P2P client
 * @NL80211_IFTYPE_P2P_GO: P2P group owner
 * @NL80211_IFTYPE_MAX: highest interface type number currently defined
 * @NUM_NL80211_IFTYPES: number of defined interface types
 *
 * These values are used with the %NL80211_ATTR_IFTYPE
 * to set the type of an interface.
 *
 */
enum nl80211_iftype {
	NL80211_IFTYPE_UNSPECIFIED,
	NL80211_IFTYPE_ADHOC,
	NL80211_IFTYPE_STATION,
	NL80211_IFTYPE_AP,
	NL80211_IFTYPE_AP_VLAN,
	NL80211_IFTYPE_WDS,
	NL80211_IFTYPE_MONITOR,
	NL80211_IFTYPE_MESH_POINT,
	NL80211_IFTYPE_P2P_CLIENT,
	NL80211_IFTYPE_P2P_GO,

	/* keep last */
	NUM_NL80211_IFTYPES,
	NL80211_IFTYPE_MAX = NUM_NL80211_IFTYPES - 1
};

/**
 * enum nl80211_sta_flags - station flags
 *
 * Station flags. When a station is added to an AP interface, it is
 * assumed to be already associated (and hence authenticated.)
 *
 * @__NL80211_STA_FLAG_INVALID: attribute number 0 is reserved
 * @NL80211_STA_FLAG_AUTHORIZED: station is authorized (802.1X)
 * @NL80211_STA_FLAG_SHORT_PREAMBLE: station is capable of receiving frames
 *	with short barker preamble
 * @NL80211_STA_FLAG_WME: station is WME/QoS capable
 * @NL80211_STA_FLAG_MFP: station uses management frame protection
 * @NL80211_STA_FLAG_AUTHENTICATED: station is authenticated
 * @NL80211_STA_FLAG_TDLS_PEER: station is a TDLS peer -- this flag should
 *	only be used in managed mode (even in the flags mask). Note that the
 *	flag can't be changed, it is only valid while adding a station, and
 *	attempts to change it will silently be ignored (rather than rejected
 *	as errors.)
 * @NL80211_STA_FLAG_MAX: highest station flag number currently defined
 * @__NL80211_STA_FLAG_AFTER_LAST: internal use
 */
enum nl80211_sta_flags {
	__NL80211_STA_FLAG_INVALID,
	NL80211_STA_FLAG_AUTHORIZED,
	NL80211_STA_FLAG_SHORT_PREAMBLE,
	NL80211_STA_FLAG_WME,
	NL80211_STA_FLAG_MFP,
	NL80211_STA_FLAG_AUTHENTICATED,
	NL80211_STA_FLAG_TDLS_PEER,

	/* keep last */
	__NL80211_STA_FLAG_AFTER_LAST,
	NL80211_STA_FLAG_MAX = __NL80211_STA_FLAG_AFTER_LAST - 1
};

/**
 * struct nl80211_sta_flag_update - station flags mask/set
 * @mask: mask of station flags to set
 * @set: which values to set them to
 *
 * Both mask and set contain bits as per &enum nl80211_sta_flags.
 */
struct nl80211_sta_flag_update {
	__u32 mask;
	__u32 set;
} __attribute__((packed));

/**
 * enum nl80211_rate_info - bitrate information
 *
 * These attribute types are used with %NL80211_STA_INFO_TXRATE
 * when getting information about the bitrate of a station.
 *
 * @__NL80211_RATE_INFO_INVALID: attribute number 0 is reserved
 * @NL80211_RATE_INFO_BITRATE: total bitrate (u16, 100kbit/s)
 * @NL80211_RATE_INFO_MCS: mcs index for 802.11n (u8)
 * @NL80211_RATE_INFO_40_MHZ_WIDTH: 40 Mhz dualchannel bitrate
 * @NL80211_RATE_INFO_SHORT_GI: 400ns guard interval
 * @NL80211_RATE_INFO_BITRATE32: total bitrate (u32, 100kbit/s)
 * @NL80211_RATE_INFO_MAX: highest rate_info number currently defined
 * @NL80211_RATE_INFO_VHT_MCS: MCS index for VHT (u8)
 * @NL80211_RATE_INFO_VHT_NSS: number of streams in VHT (u8)
 * @NL80211_RATE_INFO_80_MHZ_WIDTH: 80 MHz VHT rate
 * @NL80211_RATE_INFO_80P80_MHZ_WIDTH: unused - 80+80 is treated the
 *	same as 160 for purposes of the bitrates
 * @NL80211_RATE_INFO_160_MHZ_WIDTH: 160 MHz VHT rate
 * @NL80211_RATE_INFO_10_MHZ_WIDTH: 10 MHz width - note that this is
 *	a legacy rate and will be reported as the actual bitrate, i.e.
 *	half the base (20 MHz) rate
 * @NL80211_RATE_INFO_5_MHZ_WIDTH: 5 MHz width - note that this is
 *	a legacy rate and will be reported as the actual bitrate, i.e.
 *	a quarter of the base (20 MHz) rate
 * @NL80211_RATE_INFO_HE_MCS: HE MCS index (u8, 0-11)
 * @NL80211_RATE_INFO_HE_NSS: HE NSS value (u8, 1-8)
 * @NL80211_RATE_INFO_HE_GI: HE guard interval identifier
 *	(u8, see &enum nl80211_he_gi)
 * @NL80211_RATE_INFO_HE_DCM: HE DCM value (u8, 0/1)
 * @NL80211_RATE_INFO_RU_ALLOC: HE RU allocation, if not present then
 * @NL80211_RATE_INFO_MAX: highest rate_info number currently defined
 * @__NL80211_RATE_INFO_AFTER_LAST: internal use
 */
enum nl80211_rate_info {
	__NL80211_RATE_INFO_INVALID,
	NL80211_RATE_INFO_BITRATE,
	NL80211_RATE_INFO_MCS,
	NL80211_RATE_INFO_40_MHZ_WIDTH,
	NL80211_RATE_INFO_SHORT_GI,
	NL80211_RATE_INFO_BITRATE32,
	NL80211_RATE_INFO_VHT_MCS,
	NL80211_RATE_INFO_VHT_NSS,
	NL80211_RATE_INFO_80_MHZ_WIDTH,
	NL80211_RATE_INFO_80P80_MHZ_WIDTH,
	NL80211_RATE_INFO_160_MHZ_WIDTH,
	NL80211_RATE_INFO_10_MHZ_WIDTH,
	NL80211_RATE_INFO_5_MHZ_WIDTH,
	NL80211_RATE_INFO_HE_MCS,
	NL80211_RATE_INFO_HE_NSS,
	NL80211_RATE_INFO_HE_GI,
	NL80211_RATE_INFO_HE_DCM,
	NL80211_RATE_INFO_HE_RU_ALLOC,

	/* keep last */
	__NL80211_RATE_INFO_AFTER_LAST,
	NL80211_RATE_INFO_MAX = __NL80211_RATE_INFO_AFTER_LAST - 1
};

/**
 * enum nl80211_sta_bss_param - BSS information collected by STA
 *
 * These attribute types are used with %NL80211_STA_INFO_BSS_PARAM
 * when getting information about the bitrate of a station.
 *
 * @__NL80211_STA_BSS_PARAM_INVALID: attribute number 0 is reserved
 * @NL80211_STA_BSS_PARAM_CTS_PROT: whether CTS protection is enabled (flag)
 * @NL80211_STA_BSS_PARAM_SHORT_PREAMBLE:  whether short preamble is enabled
 *	(flag)
 * @NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME:  whether short slot time is enabled
 *	(flag)
 * @NL80211_STA_BSS_PARAM_DTIM_PERIOD: DTIM period for beaconing (u8)
 * @NL80211_STA_BSS_PARAM_BEACON_INTERVAL: Beacon interval (u16)
 * @NL80211_STA_BSS_PARAM_MAX: highest sta_bss_param number currently defined
 * @__NL80211_STA_BSS_PARAM_AFTER_LAST: internal use
 */
enum nl80211_sta_bss_param {
	__NL80211_STA_BSS_PARAM_INVALID,
	NL80211_STA_BSS_PARAM_CTS_PROT,
	NL80211_STA_BSS_PARAM_SHORT_PREAMBLE,
	NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME,
	NL80211_STA_BSS_PARAM_DTIM_PERIOD,
	NL80211_STA_BSS_PARAM_BEACON_INTERVAL,

	/* keep last */
	__NL80211_STA_BSS_PARAM_AFTER_LAST,
	NL80211_STA_BSS_PARAM_MAX = __NL80211_STA_BSS_PARAM_AFTER_LAST - 1
};

/**
 * enum nl80211_sta_info - station information
 *
 * These attribute types are used with %NL80211_ATTR_STA_INFO
 * when getting information about a station.
 *
 * @__NL80211_STA_INFO_INVALID: attribute number 0 is reserved
 * @NL80211_STA_INFO_INACTIVE_TIME: time since last activity (u32, msecs)
 * @NL80211_STA_INFO_RX_BYTES: total received bytes (u32, from this station)
 * @NL80211_STA_INFO_TX_BYTES: total transmitted bytes (u32, to this station)
 * @NL80211_STA_INFO_SIGNAL: signal strength of last received PPDU (u8, dBm)
 * @NL80211_STA_INFO_TX_BITRATE: current unicast tx rate, nested attribute
 * 	containing info as possible, see &enum nl80211_rate_info
 * @NL80211_STA_INFO_RX_PACKETS: total received packet (u32, from this station)
 * @NL80211_STA_INFO_TX_PACKETS: total transmitted packets (u32, to this
 *	station)
 * @NL80211_STA_INFO_TX_RETRIES: total retries (u32, to this station)
 * @NL80211_STA_INFO_TX_FAILED: total failed packets (u32, to this station)
 * @NL80211_STA_INFO_SIGNAL_AVG: signal strength average (u8, dBm)
 * @NL80211_STA_INFO_LLID: the station's mesh LLID
 * @NL80211_STA_INFO_PLID: the station's mesh PLID
 * @NL80211_STA_INFO_PLINK_STATE: peer link state for the station
 *	(see %enum nl80211_plink_state)
 * @NL80211_STA_INFO_RX_BITRATE: last unicast data frame rx rate, nested
 *	attribute, like NL80211_STA_INFO_TX_BITRATE.
 * @NL80211_STA_INFO_BSS_PARAM: current station's view of BSS, nested attribute
 *     containing info as possible, see &enum nl80211_sta_bss_param
 * @NL80211_STA_INFO_CONNECTED_TIME: time since the station is last connected
 * @NL80211_STA_INFO_STA_FLAGS: Contains a struct nl80211_sta_flag_update.
 * @NL80211_STA_INFO_BEACON_LOSS: count of times beacon loss was detected (u32)
 * @NL80211_STA_INFO_CHAIN_SIGNAL: per-chain signal strength of last PPDU
 * @NL80211_STA_INFO_CHAIN_SIGNAL_AVG: per-chain signal strength average
 * @__NL80211_STA_INFO_AFTER_LAST: internal
 * @NL80211_STA_INFO_MAX: highest possible station info attribute
 */
enum nl80211_sta_info {
	__NL80211_STA_INFO_INVALID,
	NL80211_STA_INFO_INACTIVE_TIME,
	NL80211_STA_INFO_RX_BYTES,
	NL80211_STA_INFO_TX_BYTES,
	NL80211_STA_INFO_LLID,
	NL80211_STA_INFO_PLID,
	NL80211_STA_INFO_PLINK_STATE,
	NL80211_STA_INFO_SIGNAL,
	NL80211_STA_INFO_TX_BITRATE,
	NL80211_STA_INFO_RX_PACKETS,
	NL80211_STA_INFO_TX_PACKETS,
	NL80211_STA_INFO_TX_RETRIES,
	NL80211_STA_INFO_TX_FAILED,
	NL80211_STA_INFO_SIGNAL_AVG,
	NL80211_STA_INFO_RX_BITRATE,
	NL80211_STA_INFO_BSS_PARAM,
	NL80211_STA_INFO_CONNECTED_TIME,
	NL80211_STA_INFO_STA_FLAGS,
	NL80211_STA_INFO_BEACON_LOSS,
	NL80211_STA_INFO_CHAIN_SIGNAL,
	NL80211_STA_INFO_CHAIN_SIGNAL_AVG,

	/* keep last */
	__NL80211_STA_INFO_AFTER_LAST,
	NL80211_STA_INFO_MAX = __NL80211_STA_INFO_AFTER_LAST - 1
};

/**
 * enum nl80211_mpath_flags - nl80211 mesh path flags
 *
 * @NL80211_MPATH_FLAG_ACTIVE: the mesh path is active
 * @NL80211_MPATH_FLAG_RESOLVING: the mesh path discovery process is running
 * @NL80211_MPATH_FLAG_SN_VALID: the mesh path contains a valid SN
 * @NL80211_MPATH_FLAG_FIXED: the mesh path has been manually set
 * @NL80211_MPATH_FLAG_RESOLVED: the mesh path discovery process succeeded
 */
enum nl80211_mpath_flags {
	NL80211_MPATH_FLAG_ACTIVE =	1<<0,
	NL80211_MPATH_FLAG_RESOLVING =	1<<1,
	NL80211_MPATH_FLAG_SN_VALID =	1<<2,
	NL80211_MPATH_FLAG_FIXED =	1<<3,
	NL80211_MPATH_FLAG_RESOLVED =	1<<4,
};

/**
 * enum nl80211_mpath_info - mesh path information
 *
 * These attribute types are used with %NL80211_ATTR_MPATH_INFO when getting
 * information about a mesh path.
 *
 * @__NL80211_MPATH_INFO_INVALID: attribute number 0 is reserved
 * @NL80211_MPATH_INFO_FRAME_QLEN: number of queued frames for this destination
 * @NL80211_MPATH_INFO_SN: destination sequence number
 * @NL80211_MPATH_INFO_METRIC: metric (cost) of this mesh path
 * @NL80211_MPATH_INFO_EXPTIME: expiration time for the path, in msec from now
 * @NL80211_MPATH_INFO_FLAGS: mesh path flags, enumerated in
 * 	&enum nl80211_mpath_flags;
 * @NL80211_MPATH_INFO_DISCOVERY_TIMEOUT: total path discovery timeout, in msec
 * @NL80211_MPATH_INFO_DISCOVERY_RETRIES: mesh path discovery retries
 * @NL80211_MPATH_INFO_MAX: highest mesh path information attribute number
 *	currently defind
 * @__NL80211_MPATH_INFO_AFTER_LAST: internal use
 */
enum nl80211_mpath_info {
	__NL80211_MPATH_INFO_INVALID,
	NL80211_MPATH_INFO_FRAME_QLEN,
	NL80211_MPATH_INFO_SN,
	NL80211_MPATH_INFO_METRIC,
	NL80211_MPATH_INFO_EXPTIME,
	NL80211_MPATH_INFO_FLAGS,
	NL80211_MPATH_INFO_DISCOVERY_TIMEOUT,
	NL80211_MPATH_INFO_DISCOVERY_RETRIES,

	/* keep last */
	__NL80211_MPATH_INFO_AFTER_LAST,
	NL80211_MPATH_INFO_MAX = __NL80211_MPATH_INFO_AFTER_LAST - 1
};

/**
 * enum nl80211_band_attr - band attributes
 * @__NL80211_BAND_ATTR_INVALID: attribute number 0 is reserved
 * @NL80211_BAND_ATTR_FREQS: supported frequencies in this band,
 *	an array of nested frequency attributes
 * @NL80211_BAND_ATTR_RATES: supported bitrates in this band,
 *	an array of nested bitrate attributes
 * @NL80211_BAND_ATTR_HT_MCS_SET: 16-byte attribute containing the MCS set as
 *	defined in 802.11n
 * @NL80211_BAND_ATTR_HT_CAPA: HT capabilities, as in the HT information IE
 * @NL80211_BAND_ATTR_HT_AMPDU_FACTOR: A-MPDU factor, as in 11n
 * @NL80211_BAND_ATTR_HT_AMPDU_DENSITY: A-MPDU density, as in 11n
 * @NL80211_BAND_ATTR_MAX: highest band attribute currently defined
 * @__NL80211_BAND_ATTR_AFTER_LAST: internal use
 */
enum nl80211_band_attr {
	__NL80211_BAND_ATTR_INVALID,
	NL80211_BAND_ATTR_FREQS,
	NL80211_BAND_ATTR_RATES,

	NL80211_BAND_ATTR_HT_MCS_SET,
	NL80211_BAND_ATTR_HT_CAPA,
	NL80211_BAND_ATTR_HT_AMPDU_FACTOR,
	NL80211_BAND_ATTR_HT_AMPDU_DENSITY,
		
	NL80211_BAND_ATTR_VHT_MCS_SET,
	NL80211_BAND_ATTR_VHT_CAPA,
	NL80211_BAND_ATTR_IFTYPE_DATA,
		
	NL80211_BAND_ATTR_EDMG_CHANNELS,
	NL80211_BAND_ATTR_EDMG_BW_CONFIG,

	/* keep last */
	__NL80211_BAND_ATTR_AFTER_LAST,
	NL80211_BAND_ATTR_MAX = __NL80211_BAND_ATTR_AFTER_LAST - 1
};

#define NL80211_BAND_ATTR_HT_CAPA NL80211_BAND_ATTR_HT_CAPA

/**
 * enum nl80211_band_iftype_attr - Interface type data attributes
 *
 * @__NL80211_BAND_IFTYPE_ATTR_INVALID: attribute number 0 is reserved
 * @NL80211_BAND_IFTYPE_ATTR_IFTYPES: nested attribute containing a flag attribute
 *     for each interface type that supports the band data
 * @NL80211_BAND_IFTYPE_ATTR_HE_CAP_MAC: HE MAC capabilities as in HE
 *     capabilities IE
 * @NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY: HE PHY capabilities as in HE
 *     capabilities IE
 * @NL80211_BAND_IFTYPE_ATTR_HE_CAP_MCS_SET: HE supported NSS/MCS as in HE
 *     capabilities IE
 * @NL80211_BAND_IFTYPE_ATTR_HE_CAP_PPE: HE PPE thresholds information as
 *     defined in HE capabilities IE
 * @NL80211_BAND_IFTYPE_ATTR_EHT_CAP_MAC: EHT MAC capabilities as in EHT
 *     capabilities IE
 * @NL80211_BAND_IFTYPE_ATTR_EHT_CAP_PHY: EHT PHY capabilities as in EHT
 *     capabilities IE
 * @NL80211_BAND_IFTYPE_ATTR_EHT_CAP_MCS_SET: EHT supported NSS/MCS as in EHT
 *     capabilities IE
 * @NL80211_BAND_IFTYPE_ATTR_MAX: highest band HE capability attribute currently
 *     defined
 * @__NL80211_BAND_IFTYPE_ATTR_AFTER_LAST: internal use
 */
enum nl80211_band_iftype_attr {
	__NL80211_BAND_IFTYPE_ATTR_INVALID,

	NL80211_BAND_IFTYPE_ATTR_IFTYPES,
	NL80211_BAND_IFTYPE_ATTR_HE_CAP_MAC,
	NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY,
	NL80211_BAND_IFTYPE_ATTR_HE_CAP_MCS_SET,
	NL80211_BAND_IFTYPE_ATTR_HE_CAP_PPE,
	NL80211_BAND_IFTYPE_ATTR_HE_6GHZ_CAPA,
	NL80211_BAND_IFTYPE_ATTR_EHT_CAP_MAC,
	NL80211_BAND_IFTYPE_ATTR_EHT_CAP_PHY,
	NL80211_BAND_IFTYPE_ATTR_EHT_CAP_MCS_SET,

	/* keep last */
	__NL80211_BAND_IFTYPE_ATTR_AFTER_LAST,
	NL80211_BAND_IFTYPE_ATTR_MAX = __NL80211_BAND_IFTYPE_ATTR_AFTER_LAST - 1
};

/**
 * enum nl80211_frequency_attr - frequency attributes
 * @__NL80211_FREQUENCY_ATTR_INVALID: attribute number 0 is reserved
 * @NL80211_FREQUENCY_ATTR_FREQ: Frequency in MHz
 * @NL80211_FREQUENCY_ATTR_DISABLED: Channel is disabled in current
 *	regulatory domain.
 * @NL80211_FREQUENCY_ATTR_PASSIVE_SCAN: Only passive scanning is
 *	permitted on this channel in current regulatory domain.
 * @NL80211_FREQUENCY_ATTR_NO_IBSS: IBSS networks are not permitted
 *	on this channel in current regulatory domain.
 * @NL80211_FREQUENCY_ATTR_RADAR: Radar detection is mandatory
 *	on this channel in current regulatory domain.
 * @NL80211_FREQUENCY_ATTR_MAX_TX_POWER: Maximum transmission power in mBm
 *	(100 * dBm).
 * @NL80211_FREQUENCY_ATTR_MAX: highest frequency attribute number
 *	currently defined
 * @__NL80211_FREQUENCY_ATTR_AFTER_LAST: internal use
 */
enum nl80211_frequency_attr {
	__NL80211_FREQUENCY_ATTR_INVALID,
	NL80211_FREQUENCY_ATTR_FREQ,
	NL80211_FREQUENCY_ATTR_DISABLED,
	NL80211_FREQUENCY_ATTR_NO_IR,
	__NL80211_FREQUENCY_ATTR_NO_IBSS,
	NL80211_FREQUENCY_ATTR_RADAR,
	NL80211_FREQUENCY_ATTR_MAX_TX_POWER,
	NL80211_FREQUENCY_ATTR_DFS_STATE,
	NL80211_FREQUENCY_ATTR_DFS_TIME,
	NL80211_FREQUENCY_ATTR_NO_HT40_MINUS,
	NL80211_FREQUENCY_ATTR_NO_HT40_PLUS,
	NL80211_FREQUENCY_ATTR_NO_80MHZ,
	NL80211_FREQUENCY_ATTR_NO_160MHZ,
	NL80211_FREQUENCY_ATTR_DFS_CAC_TIME,
	NL80211_FREQUENCY_ATTR_INDOOR_ONLY,
	NL80211_FREQUENCY_ATTR_IR_CONCURRENT,
	NL80211_FREQUENCY_ATTR_NO_20MHZ,
	NL80211_FREQUENCY_ATTR_NO_10MHZ,
	NL80211_FREQUENCY_ATTR_WMM,
	NL80211_FREQUENCY_ATTR_CHANNEL,

	/* keep last */
	__NL80211_FREQUENCY_ATTR_AFTER_LAST,
	NL80211_FREQUENCY_ATTR_MAX = __NL80211_FREQUENCY_ATTR_AFTER_LAST - 1
};

#define NL80211_FREQUENCY_ATTR_MAX_TX_POWER NL80211_FREQUENCY_ATTR_MAX_TX_POWER
#define NL80211_FREQUENCY_ATTR_PASSIVE_SCAN	NL80211_FREQUENCY_ATTR_NO_IR
#define NL80211_FREQUENCY_ATTR_NO_IBSS		NL80211_FREQUENCY_ATTR_NO_IR
#define NL80211_FREQUENCY_ATTR_NO_IR		NL80211_FREQUENCY_ATTR_NO_IR
#define NL80211_FREQUENCY_ATTR_GO_CONCURRENT \
					NL80211_FREQUENCY_ATTR_IR_CONCURRENT

/**
 * enum nl80211_bitrate_attr - bitrate attributes
 * @__NL80211_BITRATE_ATTR_INVALID: attribute number 0 is reserved
 * @NL80211_BITRATE_ATTR_RATE: Bitrate in units of 100 kbps
 * @NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE: Short preamble supported
 *	in 2.4 GHz band.
 * @NL80211_BITRATE_ATTR_MAX: highest bitrate attribute number
 *	currently defined
 * @__NL80211_BITRATE_ATTR_AFTER_LAST: internal use
 */
enum nl80211_bitrate_attr {
	__NL80211_BITRATE_ATTR_INVALID,
	NL80211_BITRATE_ATTR_RATE,
	NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE,

	/* keep last */
	__NL80211_BITRATE_ATTR_AFTER_LAST,
	NL80211_BITRATE_ATTR_MAX = __NL80211_BITRATE_ATTR_AFTER_LAST - 1
};

/**
 * enum nl80211_initiator - Indicates the initiator of a reg domain request
 * @NL80211_REGDOM_SET_BY_CORE: Core queried CRDA for a dynamic world
 * 	regulatory domain.
 * @NL80211_REGDOM_SET_BY_USER: User asked the wireless core to set the
 * 	regulatory domain.
 * @NL80211_REGDOM_SET_BY_DRIVER: a wireless drivers has hinted to the
 * 	wireless core it thinks its knows the regulatory domain we should be in.
 * @NL80211_REGDOM_SET_BY_COUNTRY_IE: the wireless core has received an
 * 	802.11 country information element with regulatory information it
 * 	thinks we should consider. cfg80211 only processes the country
 *	code from the IE, and relies on the regulatory domain information
 *	structure passed by userspace (CRDA) from our wireless-regdb.
 *	If a channel is enabled but the country code indicates it should
 *	be disabled we disable the channel and re-enable it upon disassociation.
 */
enum nl80211_reg_initiator {
	NL80211_REGDOM_SET_BY_CORE,
	NL80211_REGDOM_SET_BY_USER,
	NL80211_REGDOM_SET_BY_DRIVER,
	NL80211_REGDOM_SET_BY_COUNTRY_IE,
};

/**
 * enum nl80211_reg_type - specifies the type of regulatory domain
 * @NL80211_REGDOM_TYPE_COUNTRY: the regulatory domain set is one that pertains
 *	to a specific country. When this is set you can count on the
 *	ISO / IEC 3166 alpha2 country code being valid.
 * @NL80211_REGDOM_TYPE_WORLD: the regulatory set domain is the world regulatory
 * 	domain.
 * @NL80211_REGDOM_TYPE_CUSTOM_WORLD: the regulatory domain set is a custom
 * 	driver specific world regulatory domain. These do not apply system-wide
 * 	and are only applicable to the individual devices which have requested
 * 	them to be applied.
 * @NL80211_REGDOM_TYPE_INTERSECTION: the regulatory domain set is the product
 *	of an intersection between two regulatory domains -- the previously
 *	set regulatory domain on the system and the last accepted regulatory
 *	domain request to be processed.
 */
enum nl80211_reg_type {
	NL80211_REGDOM_TYPE_COUNTRY,
	NL80211_REGDOM_TYPE_WORLD,
	NL80211_REGDOM_TYPE_CUSTOM_WORLD,
	NL80211_REGDOM_TYPE_INTERSECTION,
};

/**
 * enum nl80211_reg_rule_attr - regulatory rule attributes
 * @__NL80211_REG_RULE_ATTR_INVALID: attribute number 0 is reserved
 * @NL80211_ATTR_REG_RULE_FLAGS: a set of flags which specify additional
 * 	considerations for a given frequency range. These are the
 * 	&enum nl80211_reg_rule_flags.
 * @NL80211_ATTR_FREQ_RANGE_START: starting frequencry for the regulatory
 * 	rule in KHz. This is not a center of frequency but an actual regulatory
 * 	band edge.
 * @NL80211_ATTR_FREQ_RANGE_END: ending frequency for the regulatory rule
 * 	in KHz. This is not a center a frequency but an actual regulatory
 * 	band edge.
 * @NL80211_ATTR_FREQ_RANGE_MAX_BW: maximum allowed bandwidth for this
 * 	frequency range, in KHz.
 * @NL80211_ATTR_POWER_RULE_MAX_ANT_GAIN: the maximum allowed antenna gain
 * 	for a given frequency range. The value is in mBi (100 * dBi).
 * 	If you don't have one then don't send this.
 * @NL80211_ATTR_POWER_RULE_MAX_EIRP: the maximum allowed EIRP for
 * 	a given frequency range. The value is in mBm (100 * dBm).
 * @NL80211_REG_RULE_ATTR_MAX: highest regulatory rule attribute number
 *	currently defined
 * @__NL80211_REG_RULE_ATTR_AFTER_LAST: internal use
 */
enum nl80211_reg_rule_attr {
	__NL80211_REG_RULE_ATTR_INVALID,
	NL80211_ATTR_REG_RULE_FLAGS,

	NL80211_ATTR_FREQ_RANGE_START,
	NL80211_ATTR_FREQ_RANGE_END,
	NL80211_ATTR_FREQ_RANGE_MAX_BW,

	NL80211_ATTR_POWER_RULE_MAX_ANT_GAIN,
	NL80211_ATTR_POWER_RULE_MAX_EIRP,

	/* keep last */
	__NL80211_REG_RULE_ATTR_AFTER_LAST,
	NL80211_REG_RULE_ATTR_MAX = __NL80211_REG_RULE_ATTR_AFTER_LAST - 1
};

/**
 * enum nl80211_sched_scan_match_attr - scheduled scan match attributes
 * @__NL80211_SCHED_SCAN_MATCH_ATTR_INVALID: attribute number 0 is reserved
 * @NL80211_SCHED_SCAN_MATCH_ATTR_SSID: SSID to be used for matching,
 * only report BSS with matching SSID.
 * @NL80211_SCHED_SCAN_MATCH_ATTR_MAX: highest scheduled scan filter
 *	attribute number currently defined
 * @__NL80211_SCHED_SCAN_MATCH_ATTR_AFTER_LAST: internal use
 */
enum nl80211_sched_scan_match_attr {
	__NL80211_SCHED_SCAN_MATCH_ATTR_INVALID,

	NL80211_ATTR_SCHED_SCAN_MATCH_SSID,

	/* keep last */
	__NL80211_SCHED_SCAN_MATCH_ATTR_AFTER_LAST,
	NL80211_SCHED_SCAN_MATCH_ATTR_MAX =
		__NL80211_SCHED_SCAN_MATCH_ATTR_AFTER_LAST - 1
};

/**
 * enum nl80211_reg_rule_flags - regulatory rule flags
 *
 * @NL80211_RRF_NO_OFDM: OFDM modulation not allowed
 * @NL80211_RRF_NO_CCK: CCK modulation not allowed
 * @NL80211_RRF_NO_INDOOR: indoor operation not allowed
 * @NL80211_RRF_NO_OUTDOOR: outdoor operation not allowed
 * @NL80211_RRF_DFS: DFS support is required to be used
 * @NL80211_RRF_PTP_ONLY: this is only for Point To Point links
 * @NL80211_RRF_PTMP_ONLY: this is only for Point To Multi Point links
 * @NL80211_RRF_PASSIVE_SCAN: passive scan is required
 * @NL80211_RRF_NO_IBSS: no IBSS is allowed
 */
enum nl80211_reg_rule_flags {
	NL80211_RRF_NO_OFDM		= 1<<0,
	NL80211_RRF_NO_CCK		= 1<<1,
	NL80211_RRF_NO_INDOOR		= 1<<2,
	NL80211_RRF_NO_OUTDOOR		= 1<<3,
	NL80211_RRF_DFS			= 1<<4,
	NL80211_RRF_PTP_ONLY		= 1<<5,
	NL80211_RRF_PTMP_ONLY		= 1<<6,
	NL80211_RRF_PASSIVE_SCAN	= 1<<7,
	NL80211_RRF_NO_IBSS		= 1<<8,
};

/**
 * enum nl80211_dfs_regions - regulatory DFS regions
 *
 * @NL80211_DFS_UNSET: Country has no DFS master region specified
 * @NL80211_DFS_FCC_: Country follows DFS master rules from FCC
 * @NL80211_DFS_FCC_: Country follows DFS master rules from ETSI
 * @NL80211_DFS_JP_: Country follows DFS master rules from JP/MKK/Telec
 */
enum nl80211_dfs_regions {
	NL80211_DFS_UNSET	= 0,
	NL80211_DFS_FCC		= 1,
	NL80211_DFS_ETSI	= 2,
	NL80211_DFS_JP		= 3,
};

/**
 * enum nl80211_survey_info - survey information
 *
 * These attribute types are used with %NL80211_ATTR_SURVEY_INFO
 * when getting information about a survey.
 *
 * @__NL80211_SURVEY_INFO_INVALID: attribute number 0 is reserved
 * @NL80211_SURVEY_INFO_FREQUENCY: center frequency of channel
 * @NL80211_SURVEY_INFO_NOISE: noise level of channel (u8, dBm)
 * @NL80211_SURVEY_INFO_IN_USE: channel is currently being used
 * @NL80211_SURVEY_INFO_CHANNEL_TIME: amount of time (in ms) that the radio
 *	spent on this channel
 * @NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY: amount of the time the primary
 *	channel was sensed busy (either due to activity or energy detect)
 * @NL80211_SURVEY_INFO_CHANNEL_TIME_EXT_BUSY: amount of time the extension
 *	channel was sensed busy
 * @NL80211_SURVEY_INFO_CHANNEL_TIME_RX: amount of time the radio spent
 *	receiving data
 * @NL80211_SURVEY_INFO_CHANNEL_TIME_TX: amount of time the radio spent
 *	transmitting data
 * @NL80211_SURVEY_INFO_MAX: highest survey info attribute number
 *	currently defined
 * @__NL80211_SURVEY_INFO_AFTER_LAST: internal use
 */
enum nl80211_survey_info {
	__NL80211_SURVEY_INFO_INVALID,
	NL80211_SURVEY_INFO_FREQUENCY,
	NL80211_SURVEY_INFO_NOISE,
	NL80211_SURVEY_INFO_IN_USE,
	NL80211_SURVEY_INFO_CHANNEL_TIME,
	NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY,
	NL80211_SURVEY_INFO_CHANNEL_TIME_EXT_BUSY,
	NL80211_SURVEY_INFO_CHANNEL_TIME_RX,
	NL80211_SURVEY_INFO_CHANNEL_TIME_TX,

	/* keep last */
	__NL80211_SURVEY_INFO_AFTER_LAST,
	NL80211_SURVEY_INFO_MAX = __NL80211_SURVEY_INFO_AFTER_LAST - 1
};

/**
 * enum nl80211_mntr_flags - monitor configuration flags
 *
 * Monitor configuration flags.
 *
 * @__NL80211_MNTR_FLAG_INVALID: reserved
 *
 * @NL80211_MNTR_FLAG_FCSFAIL: pass frames with bad FCS
 * @NL80211_MNTR_FLAG_PLCPFAIL: pass frames with bad PLCP
 * @NL80211_MNTR_FLAG_CONTROL: pass control frames
 * @NL80211_MNTR_FLAG_OTHER_BSS: disable BSSID filtering
 * @NL80211_MNTR_FLAG_COOK_FRAMES: report frames after processing.
 *	overrides all other flags.
 *
 * @__NL80211_MNTR_FLAG_AFTER_LAST: internal use
 * @NL80211_MNTR_FLAG_MAX: highest possible monitor flag
 */
enum nl80211_mntr_flags {
	__NL80211_MNTR_FLAG_INVALID,
	NL80211_MNTR_FLAG_FCSFAIL,
	NL80211_MNTR_FLAG_PLCPFAIL,
	NL80211_MNTR_FLAG_CONTROL,
	NL80211_MNTR_FLAG_OTHER_BSS,
	NL80211_MNTR_FLAG_COOK_FRAMES,

	/* keep last */
	__NL80211_MNTR_FLAG_AFTER_LAST,
	NL80211_MNTR_FLAG_MAX = __NL80211_MNTR_FLAG_AFTER_LAST - 1
};

/**
 * enum nl80211_meshconf_params - mesh configuration parameters
 *
 * Mesh configuration parameters. These can be changed while the mesh is
 * active.
 *
 * @__NL80211_MESHCONF_INVALID: internal use
 *
 * @NL80211_MESHCONF_RETRY_TIMEOUT: specifies the initial retry timeout in
 * millisecond units, used by the Peer Link Open message
 *
 * @NL80211_MESHCONF_CONFIRM_TIMEOUT: specifies the initial confirm timeout, in
 * millisecond units, used by the peer link management to close a peer link
 *
 * @NL80211_MESHCONF_HOLDING_TIMEOUT: specifies the holding timeout, in
 * millisecond units
 *
 * @NL80211_MESHCONF_MAX_PEER_LINKS: maximum number of peer links allowed
 * on this mesh interface
 *
 * @NL80211_MESHCONF_MAX_RETRIES: specifies the maximum number of peer link
 * open retries that can be sent to establish a new peer link instance in a
 * mesh
 *
 * @NL80211_MESHCONF_TTL: specifies the value of TTL field set at a source mesh
 * point.
 *
 * @NL80211_MESHCONF_AUTO_OPEN_PLINKS: whether we should automatically
 * open peer links when we detect compatible mesh peers.
 *
 * @NL80211_MESHCONF_HWMP_MAX_PREQ_RETRIES: the number of action frames
 * containing a PREQ that an MP can send to a particular destination (path
 * target)
 *
 * @NL80211_MESHCONF_PATH_REFRESH_TIME: how frequently to refresh mesh paths
 * (in milliseconds)
 *
 * @NL80211_MESHCONF_MIN_DISCOVERY_TIMEOUT: minimum length of time to wait
 * until giving up on a path discovery (in milliseconds)
 *
 * @NL80211_MESHCONF_HWMP_ACTIVE_PATH_TIMEOUT: The time (in TUs) for which mesh
 * points receiving a PREQ shall consider the forwarding information from the
 * root to be valid. (TU = time unit)
 *
 * @NL80211_MESHCONF_HWMP_PREQ_MIN_INTERVAL: The minimum interval of time (in
 * TUs) during which an MP can send only one action frame containing a PREQ
 * reference element
 *
 * @NL80211_MESHCONF_HWMP_NET_DIAM_TRVS_TIME: The interval of time (in TUs)
 * that it takes for an HWMP information element to propagate across the mesh
 *
 * @NL80211_MESHCONF_HWMP_ROOTMODE: whether root mode is enabled or not
 *
 * @NL80211_MESHCONF_ELEMENT_TTL: specifies the value of TTL field set at a
 * source mesh point for path selection elements.
 *
 * @NL80211_MESHCONF_HWMP_RANN_INTERVAL:  The interval of time (in TUs) between
 * root announcements are transmitted.
 *
 * @NL80211_MESHCONF_GATE_ANNOUNCEMENTS: Advertise that this mesh station has
 * access to a broader network beyond the MBSS.  This is done via Root
 * Announcement frames.
 *
 * @NL80211_MESHCONF_HWMP_PERR_MIN_INTERVAL: The minimum interval of time (in
 * TUs) during which a mesh STA can send only one Action frame containing a
 * PERR element.
 *
 * @NL80211_MESHCONF_FORWARDING: set Mesh STA as forwarding or non-forwarding
 * or forwarding entity (default is TRUE - forwarding entity)
 *
 * @NL80211_MESHCONF_ATTR_MAX: highest possible mesh configuration attribute
 *
 * @__NL80211_MESHCONF_ATTR_AFTER_LAST: internal use
 */
enum nl80211_meshconf_params {
	__NL80211_MESHCONF_INVALID,
	NL80211_MESHCONF_RETRY_TIMEOUT,
	NL80211_MESHCONF_CONFIRM_TIMEOUT,
	NL80211_MESHCONF_HOLDING_TIMEOUT,
	NL80211_MESHCONF_MAX_PEER_LINKS,
	NL80211_MESHCONF_MAX_RETRIES,
	NL80211_MESHCONF_TTL,
	NL80211_MESHCONF_AUTO_OPEN_PLINKS,
	NL80211_MESHCONF_HWMP_MAX_PREQ_RETRIES,
	NL80211_MESHCONF_PATH_REFRESH_TIME,
	NL80211_MESHCONF_MIN_DISCOVERY_TIMEOUT,
	NL80211_MESHCONF_HWMP_ACTIVE_PATH_TIMEOUT,
	NL80211_MESHCONF_HWMP_PREQ_MIN_INTERVAL,
	NL80211_MESHCONF_HWMP_NET_DIAM_TRVS_TIME,
	NL80211_MESHCONF_HWMP_ROOTMODE,
	NL80211_MESHCONF_ELEMENT_TTL,
	NL80211_MESHCONF_HWMP_RANN_INTERVAL,
	NL80211_MESHCONF_GATE_ANNOUNCEMENTS,
	NL80211_MESHCONF_HWMP_PERR_MIN_INTERVAL,
	NL80211_MESHCONF_FORWARDING,

	/* keep last */
	__NL80211_MESHCONF_ATTR_AFTER_LAST,
	NL80211_MESHCONF_ATTR_MAX = __NL80211_MESHCONF_ATTR_AFTER_LAST - 1
};

/**
 * enum nl80211_mesh_setup_params - mesh setup parameters
 *
 * Mesh setup parameters.  These are used to start/join a mesh and cannot be
 * changed while the mesh is active.
 *
 * @__NL80211_MESH_SETUP_INVALID: Internal use
 *
 * @NL80211_MESH_SETUP_ENABLE_VENDOR_PATH_SEL: Enable this option to use a
 * vendor specific path selection algorithm or disable it to use the default
 * HWMP.
 *
 * @NL80211_MESH_SETUP_ENABLE_VENDOR_METRIC: Enable this option to use a
 * vendor specific path metric or disable it to use the default Airtime
 * metric.
 *
 * @NL80211_MESH_SETUP_IE: Information elements for this mesh, for instance, a
 * robust security network ie, or a vendor specific information element that
 * vendors will use to identify the path selection methods and metrics in use.
 *
 * @NL80211_MESH_SETUP_USERSPACE_AUTH: Enable this option if an authentication
 * daemon will be authenticating mesh candidates.
 *
 * @NL80211_MESH_SETUP_USERSPACE_AMPE: Enable this option if an authentication
 * daemon will be securing peer link frames.  AMPE is a secured version of Mesh
 * Peering Management (MPM) and is implemented with the assistance of a
 * userspace daemon.  When this flag is set, the kernel will send peer
 * management frames to a userspace daemon that will implement AMPE
 * functionality (security capabilities selection, key confirmation, and key
 * management).  When the flag is unset (default), the kernel can autonomously
 * complete (unsecured) mesh peering without the need of a userspace daemon.
 *
 * @NL80211_MESH_SETUP_ATTR_MAX: highest possible mesh setup attribute number
 * @__NL80211_MESH_SETUP_ATTR_AFTER_LAST: Internal use
 */
enum nl80211_mesh_setup_params {
	__NL80211_MESH_SETUP_INVALID,
	NL80211_MESH_SETUP_ENABLE_VENDOR_PATH_SEL,
	NL80211_MESH_SETUP_ENABLE_VENDOR_METRIC,
	NL80211_MESH_SETUP_IE,
	NL80211_MESH_SETUP_USERSPACE_AUTH,
	NL80211_MESH_SETUP_USERSPACE_AMPE,

	/* keep last */
	__NL80211_MESH_SETUP_ATTR_AFTER_LAST,
	NL80211_MESH_SETUP_ATTR_MAX = __NL80211_MESH_SETUP_ATTR_AFTER_LAST - 1
};

/**
 * enum nl80211_txq_attr - TX queue parameter attributes
 * @__NL80211_TXQ_ATTR_INVALID: Attribute number 0 is reserved
 * @NL80211_TXQ_ATTR_QUEUE: TX queue identifier (NL80211_TXQ_Q_*)
 * @NL80211_TXQ_ATTR_TXOP: Maximum burst time in units of 32 usecs, 0 meaning
 *	disabled
 * @NL80211_TXQ_ATTR_CWMIN: Minimum contention window [a value of the form
 *	2^n-1 in the range 1..32767]
 * @NL80211_TXQ_ATTR_CWMAX: Maximum contention window [a value of the form
 *	2^n-1 in the range 1..32767]
 * @NL80211_TXQ_ATTR_AIFS: Arbitration interframe space [0..255]
 * @__NL80211_TXQ_ATTR_AFTER_LAST: Internal
 * @NL80211_TXQ_ATTR_MAX: Maximum TXQ attribute number
 */
enum nl80211_txq_attr {
	__NL80211_TXQ_ATTR_INVALID,
	NL80211_TXQ_ATTR_QUEUE,
	NL80211_TXQ_ATTR_TXOP,
	NL80211_TXQ_ATTR_CWMIN,
	NL80211_TXQ_ATTR_CWMAX,
	NL80211_TXQ_ATTR_AIFS,

	/* keep last */
	__NL80211_TXQ_ATTR_AFTER_LAST,
	NL80211_TXQ_ATTR_MAX = __NL80211_TXQ_ATTR_AFTER_LAST - 1
};

enum nl80211_txq_q {
	NL80211_TXQ_Q_VO,
	NL80211_TXQ_Q_VI,
	NL80211_TXQ_Q_BE,
	NL80211_TXQ_Q_BK
};

enum nl80211_channel_type {
	NL80211_CHAN_NO_HT,
	NL80211_CHAN_HT20,
	NL80211_CHAN_HT40MINUS,
	NL80211_CHAN_HT40PLUS
};

/**
 * enum nl80211_bss - netlink attributes for a BSS
 *
 * @__NL80211_BSS_INVALID: invalid
 * @NL80211_BSS_BSSID: BSSID of the BSS (6 octets)
 * @NL80211_BSS_FREQUENCY: frequency in MHz (u32)
 * @NL80211_BSS_TSF: TSF of the received probe response/beacon (u64)
 * @NL80211_BSS_BEACON_INTERVAL: beacon interval of the (I)BSS (u16)
 * @NL80211_BSS_CAPABILITY: capability field (CPU order, u16)
 * @NL80211_BSS_INFORMATION_ELEMENTS: binary attribute containing the
 *	raw information elements from the probe response/beacon (bin);
 *	if the %NL80211_BSS_BEACON_IES attribute is present, the IEs here are
 *	from a Probe Response frame; otherwise they are from a Beacon frame.
 *	However, if the driver does not indicate the source of the IEs, these
 *	IEs may be from either frame subtype.
 * @NL80211_BSS_SIGNAL_MBM: signal strength of probe response/beacon
 *	in mBm (100 * dBm) (s32)
 * @NL80211_BSS_SIGNAL_UNSPEC: signal strength of the probe response/beacon
 *	in unspecified units, scaled to 0..100 (u8)
 * @NL80211_BSS_STATUS: status, if this BSS is "used"
 * @NL80211_BSS_SEEN_MS_AGO: age of this BSS entry in ms
 * @NL80211_BSS_BEACON_IES: binary attribute containing the raw information
 *	elements from a Beacon frame (bin); not present if no Beacon frame has
 *	yet been received
 * @__NL80211_BSS_AFTER_LAST: internal
 * @NL80211_BSS_MAX: highest BSS attribute
 */
enum nl80211_bss {
	__NL80211_BSS_INVALID,
	NL80211_BSS_BSSID,
	NL80211_BSS_FREQUENCY,
	NL80211_BSS_TSF,
	NL80211_BSS_BEACON_INTERVAL,
	NL80211_BSS_CAPABILITY,
	NL80211_BSS_INFORMATION_ELEMENTS,
	NL80211_BSS_SIGNAL_MBM,
	NL80211_BSS_SIGNAL_UNSPEC,
	NL80211_BSS_STATUS,
	NL80211_BSS_SEEN_MS_AGO,
	NL80211_BSS_BEACON_IES,

	/* keep last */
	__NL80211_BSS_AFTER_LAST,
	NL80211_BSS_MAX = __NL80211_BSS_AFTER_LAST - 1
};

/**
 * enum nl80211_bss_status - BSS "status"
 * @NL80211_BSS_STATUS_AUTHENTICATED: Authenticated with this BSS.
 * @NL80211_BSS_STATUS_ASSOCIATED: Associated with this BSS.
 * @NL80211_BSS_STATUS_IBSS_JOINED: Joined to this IBSS.
 *
 * The BSS status is a BSS attribute in scan dumps, which
 * indicates the status the interface has wrt. this BSS.
 */
enum nl80211_bss_status {
	NL80211_BSS_STATUS_AUTHENTICATED,
	NL80211_BSS_STATUS_ASSOCIATED,
	NL80211_BSS_STATUS_IBSS_JOINED,
};

/**
 * enum nl80211_auth_type - AuthenticationType
 *
 * @NL80211_AUTHTYPE_OPEN_SYSTEM: Open System authentication
 * @NL80211_AUTHTYPE_SHARED_KEY: Shared Key authentication (WEP only)
 * @NL80211_AUTHTYPE_FT: Fast BSS Transition (IEEE 802.11r)
 * @NL80211_AUTHTYPE_NETWORK_EAP: Network EAP (some Cisco APs and mainly LEAP)
 * @__NL80211_AUTHTYPE_NUM: internal
 * @NL80211_AUTHTYPE_MAX: maximum valid auth algorithm
 * @NL80211_AUTHTYPE_AUTOMATIC: determine automatically (if necessary by
 *	trying multiple times); this is invalid in netlink -- leave out
 *	the attribute for this on CONNECT commands.
 */
enum nl80211_auth_type {
	NL80211_AUTHTYPE_OPEN_SYSTEM,
	NL80211_AUTHTYPE_SHARED_KEY,
	NL80211_AUTHTYPE_FT,
	NL80211_AUTHTYPE_NETWORK_EAP,

	/* keep last */
	__NL80211_AUTHTYPE_NUM,
	NL80211_AUTHTYPE_MAX = __NL80211_AUTHTYPE_NUM - 1,
	NL80211_AUTHTYPE_AUTOMATIC
};

/**
 * enum nl80211_key_type - Key Type
 * @NL80211_KEYTYPE_GROUP: Group (broadcast/multicast) key
 * @NL80211_KEYTYPE_PAIRWISE: Pairwise (unicast/individual) key
 * @NL80211_KEYTYPE_PEERKEY: PeerKey (DLS)
 * @NUM_NL80211_KEYTYPES: number of defined key types
 */
enum nl80211_key_type {
	NL80211_KEYTYPE_GROUP,
	NL80211_KEYTYPE_PAIRWISE,
	NL80211_KEYTYPE_PEERKEY,

	NUM_NL80211_KEYTYPES
};

/**
 * enum nl80211_mfp - Management frame protection state
 * @NL80211_MFP_NO: Management frame protection not used
 * @NL80211_MFP_REQUIRED: Management frame protection required
 */
enum nl80211_mfp {
	NL80211_MFP_NO,
	NL80211_MFP_REQUIRED,
};

enum nl80211_wpa_versions {
	NL80211_WPA_VERSION_1 = 1 << 0,
	NL80211_WPA_VERSION_2 = 1 << 1,
};

/**
 * enum nl80211_key_default_types - key default types
 * @__NL80211_KEY_DEFAULT_TYPE_INVALID: invalid
 * @NL80211_KEY_DEFAULT_TYPE_UNICAST: key should be used as default
 *	unicast key
 * @NL80211_KEY_DEFAULT_TYPE_MULTICAST: key should be used as default
 *	multicast key
 * @NUM_NL80211_KEY_DEFAULT_TYPES: number of default types
 */
enum nl80211_key_default_types {
	__NL80211_KEY_DEFAULT_TYPE_INVALID,
	NL80211_KEY_DEFAULT_TYPE_UNICAST,
	NL80211_KEY_DEFAULT_TYPE_MULTICAST,

	NUM_NL80211_KEY_DEFAULT_TYPES
};

/**
 * enum nl80211_key_attributes - key attributes
 * @__NL80211_KEY_INVALID: invalid
 * @NL80211_KEY_DATA: (temporal) key data; for TKIP this consists of
 *	16 bytes encryption key followed by 8 bytes each for TX and RX MIC
 *	keys
 * @NL80211_KEY_IDX: key ID (u8, 0-3)
 * @NL80211_KEY_CIPHER: key cipher suite (u32, as defined by IEEE 802.11
 *	section 7.3.2.25.1, e.g. 0x000FAC04)
 * @NL80211_KEY_SEQ: transmit key sequence number (IV/PN) for TKIP and
 *	CCMP keys, each six bytes in little endian
 * @NL80211_KEY_DEFAULT: flag indicating default key
 * @NL80211_KEY_DEFAULT_MGMT: flag indicating default management key
 * @NL80211_KEY_TYPE: the key type from enum nl80211_key_type, if not
 *	specified the default depends on whether a MAC address was
 *	given with the command using the key or not (u32)
 * @NL80211_KEY_DEFAULT_TYPES: A nested attribute containing flags
 *	attributes, specifying what a key should be set as default as.
 *	See &enum nl80211_key_default_types.
 * @__NL80211_KEY_AFTER_LAST: internal
 * @NL80211_KEY_MAX: highest key attribute
 */
enum nl80211_key_attributes {
	__NL80211_KEY_INVALID,
	NL80211_KEY_DATA,
	NL80211_KEY_IDX,
	NL80211_KEY_CIPHER,
	NL80211_KEY_SEQ,
	NL80211_KEY_DEFAULT,
	NL80211_KEY_DEFAULT_MGMT,
	NL80211_KEY_TYPE,
	NL80211_KEY_DEFAULT_TYPES,

	/* keep last */
	__NL80211_KEY_AFTER_LAST,
	NL80211_KEY_MAX = __NL80211_KEY_AFTER_LAST - 1
};

/**
 * enum nl80211_tx_rate_attributes - TX rate set attributes
 * @__NL80211_TXRATE_INVALID: invalid
 * @NL80211_TXRATE_LEGACY: Legacy (non-MCS) rates allowed for TX rate selection
 *	in an array of rates as defined in IEEE 802.11 7.3.2.2 (u8 values with
 *	1 = 500 kbps) but without the IE length restriction (at most
 *	%NL80211_MAX_SUPP_RATES in a single array).
 * @NL80211_TXRATE_MCS: HT (MCS) rates allowed for TX rate selection
 *	in an array of MCS numbers.
 * @__NL80211_TXRATE_AFTER_LAST: internal
 * @NL80211_TXRATE_MAX: highest TX rate attribute
 */
enum nl80211_tx_rate_attributes {
	__NL80211_TXRATE_INVALID,
	NL80211_TXRATE_LEGACY,
	NL80211_TXRATE_MCS,

	/* keep last */
	__NL80211_TXRATE_AFTER_LAST,
	NL80211_TXRATE_MAX = __NL80211_TXRATE_AFTER_LAST - 1
};

/**
 * enum nl80211_band - Frequency band
 * @NL80211_BAND_2GHZ: 2.4 GHz ISM band
 * @NL80211_BAND_5GHZ: around 5 GHz band (4.9 - 5.7 GHz)
 */
enum nl80211_band {
	NL80211_BAND_2GHZ,
	NL80211_BAND_5GHZ,
};

enum nl80211_ps_state {
	NL80211_PS_DISABLED,
	NL80211_PS_ENABLED,
};

/**
 * enum nl80211_attr_cqm - connection quality monitor attributes
 * @__NL80211_ATTR_CQM_INVALID: invalid
 * @NL80211_ATTR_CQM_RSSI_THOLD: RSSI threshold in dBm. This value specifies
 *	the threshold for the RSSI level at which an event will be sent. Zero
 *	to disable.
 * @NL80211_ATTR_CQM_RSSI_HYST: RSSI hysteresis in dBm. This value specifies
 *	the minimum amount the RSSI level must change after an event before a
 *	new event may be issued (to reduce effects of RSSI oscillation).
 * @NL80211_ATTR_CQM_RSSI_THRESHOLD_EVENT: RSSI threshold event
 * @NL80211_ATTR_CQM_PKT_LOSS_EVENT: a u32 value indicating that this many
 *	consecutive packets were not acknowledged by the peer
 * @__NL80211_ATTR_CQM_AFTER_LAST: internal
 * @NL80211_ATTR_CQM_MAX: highest key attribute
 */
enum nl80211_attr_cqm {
	__NL80211_ATTR_CQM_INVALID,
	NL80211_ATTR_CQM_RSSI_THOLD,
	NL80211_ATTR_CQM_RSSI_HYST,
	NL80211_ATTR_CQM_RSSI_THRESHOLD_EVENT,
	NL80211_ATTR_CQM_PKT_LOSS_EVENT,

	/* keep last */
	__NL80211_ATTR_CQM_AFTER_LAST,
	NL80211_ATTR_CQM_MAX = __NL80211_ATTR_CQM_AFTER_LAST - 1
};

/**
 * enum nl80211_cqm_rssi_threshold_event - RSSI threshold event
 * @NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW: The RSSI level is lower than the
 *      configured threshold
 * @NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH: The RSSI is higher than the
 *      configured threshold
 */
enum nl80211_cqm_rssi_threshold_event {
	NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW,
	NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH,
};


/**
 * enum nl80211_tx_power_setting - TX power adjustment
 * @NL80211_TX_POWER_AUTOMATIC: automatically determine transmit power
 * @NL80211_TX_POWER_LIMITED: limit TX power by the mBm parameter
 * @NL80211_TX_POWER_FIXED: fix TX power to the mBm parameter
 */
enum nl80211_tx_power_setting {
	NL80211_TX_POWER_AUTOMATIC,
	NL80211_TX_POWER_LIMITED,
	NL80211_TX_POWER_FIXED,
};

/**
 * enum nl80211_wowlan_packet_pattern_attr - WoWLAN packet pattern attribute
 * @__NL80211_WOWLAN_PKTPAT_INVALID: invalid number for nested attribute
 * @NL80211_WOWLAN_PKTPAT_PATTERN: the pattern, values where the mask has
 *	a zero bit are ignored
 * @NL80211_WOWLAN_PKTPAT_MASK: pattern mask, must be long enough to have
 *	a bit for each byte in the pattern. The lowest-order bit corresponds
 *	to the first byte of the pattern, but the bytes of the pattern are
 *	in a little-endian-like format, i.e. the 9th byte of the pattern
 *	corresponds to the lowest-order bit in the second byte of the mask.
 *	For example: The match 00:xx:00:00:xx:00:00:00:00:xx:xx:xx (where
 *	xx indicates "don't care") would be represented by a pattern of
 *	twelve zero bytes, and a mask of "0xed,0x07".
 *	Note that the pattern matching is done as though frames were not
 *	802.11 frames but 802.3 frames, i.e. the frame is fully unpacked
 *	first (including SNAP header unpacking) and then matched.
 * @NUM_NL80211_WOWLAN_PKTPAT: number of attributes
 * @MAX_NL80211_WOWLAN_PKTPAT: max attribute number
 */
enum nl80211_wowlan_packet_pattern_attr {
	__NL80211_WOWLAN_PKTPAT_INVALID,
	NL80211_WOWLAN_PKTPAT_MASK,
	NL80211_WOWLAN_PKTPAT_PATTERN,

	NUM_NL80211_WOWLAN_PKTPAT,
	MAX_NL80211_WOWLAN_PKTPAT = NUM_NL80211_WOWLAN_PKTPAT - 1,
};

/**
 * struct nl80211_wowlan_pattern_support - pattern support information
 * @max_patterns: maximum number of patterns supported
 * @min_pattern_len: minimum length of each pattern
 * @max_pattern_len: maximum length of each pattern
 *
 * This struct is carried in %NL80211_WOWLAN_TRIG_PKT_PATTERN when
 * that is part of %NL80211_ATTR_WOWLAN_TRIGGERS_SUPPORTED in the
 * capability information given by the kernel to userspace.
 */
struct nl80211_wowlan_pattern_support {
	__u32 max_patterns;
	__u32 min_pattern_len;
	__u32 max_pattern_len;
} __attribute__((packed));

/**
 * enum nl80211_wowlan_triggers - WoWLAN trigger definitions
 * @__NL80211_WOWLAN_TRIG_INVALID: invalid number for nested attributes
 * @NL80211_WOWLAN_TRIG_ANY: wake up on any activity, do not really put
 *	the chip into a special state -- works best with chips that have
 *	support for low-power operation already (flag)
 * @NL80211_WOWLAN_TRIG_DISCONNECT: wake up on disconnect, the way disconnect
 *	is detected is implementation-specific (flag)
 * @NL80211_WOWLAN_TRIG_MAGIC_PKT: wake up on magic packet (6x 0xff, followed
 *	by 16 repetitions of MAC addr, anywhere in payload) (flag)
 * @NL80211_WOWLAN_TRIG_PKT_PATTERN: wake up on the specified packet patterns
 *	which are passed in an array of nested attributes, each nested attribute
 *	defining a with attributes from &struct nl80211_wowlan_trig_pkt_pattern.
 *	Each pattern defines a wakeup packet. The matching is done on the MSDU,
 *	i.e. as though the packet was an 802.3 packet, so the pattern matching
 *	is done after the packet is converted to the MSDU.
 *
 *	In %NL80211_ATTR_WOWLAN_TRIGGERS_SUPPORTED, it is a binary attribute
 *	carrying a &struct nl80211_wowlan_pattern_support.
 * @NL80211_WOWLAN_TRIG_GTK_REKEY_SUPPORTED: Not a real trigger, and cannot be
 *	used when setting, used only to indicate that GTK rekeying is supported
 *	by the device (flag)
 * @NL80211_WOWLAN_TRIG_GTK_REKEY_FAILURE: wake up on GTK rekey failure (if
 *	done by the device) (flag)
 * @NL80211_WOWLAN_TRIG_EAP_IDENT_REQUEST: wake up on EAP Identity Request
 *	packet (flag)
 * @NL80211_WOWLAN_TRIG_4WAY_HANDSHAKE: wake up on 4-way handshake (flag)
 * @NL80211_WOWLAN_TRIG_RFKILL_RELEASE: wake up when rfkill is released
 *	(on devices that have rfkill in the device) (flag)
 * @NUM_NL80211_WOWLAN_TRIG: number of wake on wireless triggers
 * @MAX_NL80211_WOWLAN_TRIG: highest wowlan trigger attribute number
 */
enum nl80211_wowlan_triggers {
	__NL80211_WOWLAN_TRIG_INVALID,
	NL80211_WOWLAN_TRIG_ANY,
	NL80211_WOWLAN_TRIG_DISCONNECT,
	NL80211_WOWLAN_TRIG_MAGIC_PKT,
	NL80211_WOWLAN_TRIG_PKT_PATTERN,
	NL80211_WOWLAN_TRIG_GTK_REKEY_SUPPORTED,
	NL80211_WOWLAN_TRIG_GTK_REKEY_FAILURE,
	NL80211_WOWLAN_TRIG_EAP_IDENT_REQUEST,
	NL80211_WOWLAN_TRIG_4WAY_HANDSHAKE,
	NL80211_WOWLAN_TRIG_RFKILL_RELEASE,

	/* keep last */
	NUM_NL80211_WOWLAN_TRIG,
	MAX_NL80211_WOWLAN_TRIG = NUM_NL80211_WOWLAN_TRIG - 1
};

/**
 * enum nl80211_iface_limit_attrs - limit attributes
 * @NL80211_IFACE_LIMIT_UNSPEC: (reserved)
 * @NL80211_IFACE_LIMIT_MAX: maximum number of interfaces that
 *	can be chosen from this set of interface types (u32)
 * @NL80211_IFACE_LIMIT_TYPES: nested attribute containing a
 *	flag attribute for each interface type in this set
 * @NUM_NL80211_IFACE_LIMIT: number of attributes
 * @MAX_NL80211_IFACE_LIMIT: highest attribute number
 */
enum nl80211_iface_limit_attrs {
	NL80211_IFACE_LIMIT_UNSPEC,
	NL80211_IFACE_LIMIT_MAX,
	NL80211_IFACE_LIMIT_TYPES,

	/* keep last */
	NUM_NL80211_IFACE_LIMIT,
	MAX_NL80211_IFACE_LIMIT = NUM_NL80211_IFACE_LIMIT - 1
};

/**
 * enum nl80211_if_combination_attrs -- interface combination attributes
 *
 * @NL80211_IFACE_COMB_UNSPEC: (reserved)
 * @NL80211_IFACE_COMB_LIMITS: Nested attributes containing the limits
 *	for given interface types, see &enum nl80211_iface_limit_attrs.
 * @NL80211_IFACE_COMB_MAXNUM: u32 attribute giving the total number of
 *	interfaces that can be created in this group. This number doesn't
 *	apply to interfaces purely managed in software, which are listed
 *	in a separate attribute %NL80211_ATTR_INTERFACES_SOFTWARE.
 * @NL80211_IFACE_COMB_STA_AP_BI_MATCH: flag attribute specifying that
 *	beacon intervals within this group must be all the same even for
 *	infrastructure and AP/GO combinations, i.e. the GO(s) must adopt
 *	the infrastructure network's beacon interval.
 * @NL80211_IFACE_COMB_NUM_CHANNELS: u32 attribute specifying how many
 *	different channels may be used within this group.
 * @NUM_NL80211_IFACE_COMB: number of attributes
 * @MAX_NL80211_IFACE_COMB: highest attribute number
 *
 * Examples:
 *	limits = [ #{STA} <= 1, #{AP} <= 1 ], matching BI, channels = 1, max = 2
 *	=> allows an AP and a STA that must match BIs
 *
 *	numbers = [ #{AP, P2P-GO} <= 8 ], channels = 1, max = 8
 *	=> allows 8 of AP/GO
 *
 *	numbers = [ #{STA} <= 2 ], channels = 2, max = 2
 *	=> allows two STAs on different channels
 *
 *	numbers = [ #{STA} <= 1, #{P2P-client,P2P-GO} <= 3 ], max = 4
 *	=> allows a STA plus three P2P interfaces
 *
 * The list of these four possiblities could completely be contained
 * within the %NL80211_ATTR_INTERFACE_COMBINATIONS attribute to indicate
 * that any of these groups must match.
 *
 * "Combinations" of just a single interface will not be listed here,
 * a single interface of any valid interface type is assumed to always
 * be possible by itself. This means that implicitly, for each valid
 * interface type, the following group always exists:
 *	numbers = [ #{<type>} <= 1 ], channels = 1, max = 1
 */
enum nl80211_if_combination_attrs {
	NL80211_IFACE_COMB_UNSPEC,
	NL80211_IFACE_COMB_LIMITS,
	NL80211_IFACE_COMB_MAXNUM,
	NL80211_IFACE_COMB_STA_AP_BI_MATCH,
	NL80211_IFACE_COMB_NUM_CHANNELS,

	/* keep last */
	NUM_NL80211_IFACE_COMB,
	MAX_NL80211_IFACE_COMB = NUM_NL80211_IFACE_COMB - 1
};


/**
 * enum nl80211_plink_state - state of a mesh peer link finite state machine
 *
 * @NL80211_PLINK_LISTEN: initial state, considered the implicit
 *	state of non existant mesh peer links
 * @NL80211_PLINK_OPN_SNT: mesh plink open frame has been sent to
 *	this mesh peer
 * @NL80211_PLINK_OPN_RCVD: mesh plink open frame has been received
 *	from this mesh peer
 * @NL80211_PLINK_CNF_RCVD: mesh plink confirm frame has been
 *	received from this mesh peer
 * @NL80211_PLINK_ESTAB: mesh peer link is established
 * @NL80211_PLINK_HOLDING: mesh peer link is being closed or cancelled
 * @NL80211_PLINK_BLOCKED: all frames transmitted from this mesh
 *	plink are discarded
 * @NUM_NL80211_PLINK_STATES: number of peer link states
 * @MAX_NL80211_PLINK_STATES: highest numerical value of plink states
 */
enum nl80211_plink_state {
	NL80211_PLINK_LISTEN,
	NL80211_PLINK_OPN_SNT,
	NL80211_PLINK_OPN_RCVD,
	NL80211_PLINK_CNF_RCVD,
	NL80211_PLINK_ESTAB,
	NL80211_PLINK_HOLDING,
	NL80211_PLINK_BLOCKED,

	/* keep last */
	NUM_NL80211_PLINK_STATES,
	MAX_NL80211_PLINK_STATES = NUM_NL80211_PLINK_STATES - 1
};

#define NL80211_KCK_LEN			16
#define NL80211_KEK_LEN			16
#define NL80211_REPLAY_CTR_LEN		8

/**
 * enum nl80211_rekey_data - attributes for GTK rekey offload
 * @__NL80211_REKEY_DATA_INVALID: invalid number for nested attributes
 * @NL80211_REKEY_DATA_KEK: key encryption key (binary)
 * @NL80211_REKEY_DATA_KCK: key confirmation key (binary)
 * @NL80211_REKEY_DATA_REPLAY_CTR: replay counter (binary)
 * @NUM_NL80211_REKEY_DATA: number of rekey attributes (internal)
 * @MAX_NL80211_REKEY_DATA: highest rekey attribute (internal)
 */
enum nl80211_rekey_data {
	__NL80211_REKEY_DATA_INVALID,
	NL80211_REKEY_DATA_KEK,
	NL80211_REKEY_DATA_KCK,
	NL80211_REKEY_DATA_REPLAY_CTR,

	/* keep last */
	NUM_NL80211_REKEY_DATA,
	MAX_NL80211_REKEY_DATA = NUM_NL80211_REKEY_DATA - 1
};

/**
 * enum nl80211_hidden_ssid - values for %NL80211_ATTR_HIDDEN_SSID
 * @NL80211_HIDDEN_SSID_NOT_IN_USE: do not hide SSID (i.e., broadcast it in
 *	Beacon frames)
 * @NL80211_HIDDEN_SSID_ZERO_LEN: hide SSID by using zero-length SSID element
 *	in Beacon frames
 * @NL80211_HIDDEN_SSID_ZERO_CONTENTS: hide SSID by using correct length of SSID
 *	element in Beacon frames but zero out each byte in the SSID
 */
enum nl80211_hidden_ssid {
	NL80211_HIDDEN_SSID_NOT_IN_USE,
	NL80211_HIDDEN_SSID_ZERO_LEN,
	NL80211_HIDDEN_SSID_ZERO_CONTENTS
};

/**
 * enum nl80211_sta_wme_attr - station WME attributes
 * @__NL80211_STA_WME_INVALID: invalid number for nested attribute
 * @NL80211_STA_WME_UAPSD_QUEUES: bitmap of uapsd queues. the format
 *	is the same as the AC bitmap in the QoS info field.
 * @NL80211_STA_WME_MAX_SP: max service period. the format is the same
 *	as the MAX_SP field in the QoS info field (but already shifted down).
 * @__NL80211_STA_WME_AFTER_LAST: internal
 * @NL80211_STA_WME_MAX: highest station WME attribute
 */
enum nl80211_sta_wme_attr {
	__NL80211_STA_WME_INVALID,
	NL80211_STA_WME_UAPSD_QUEUES,
	NL80211_STA_WME_MAX_SP,

	/* keep last */
	__NL80211_STA_WME_AFTER_LAST,
	NL80211_STA_WME_MAX = __NL80211_STA_WME_AFTER_LAST - 1
};

/**
 * enum nl80211_pmksa_candidate_attr - attributes for PMKSA caching candidates
 * @__NL80211_PMKSA_CANDIDATE_INVALID: invalid number for nested attributes
 * @NL80211_PMKSA_CANDIDATE_INDEX: candidate index (u32; the smaller, the higher
 *	priority)
 * @NL80211_PMKSA_CANDIDATE_BSSID: candidate BSSID (6 octets)
 * @NL80211_PMKSA_CANDIDATE_PREAUTH: RSN pre-authentication supported (flag)
 * @NUM_NL80211_PMKSA_CANDIDATE: number of PMKSA caching candidate attributes
 *	(internal)
 * @MAX_NL80211_PMKSA_CANDIDATE: highest PMKSA caching candidate attribute
 *	(internal)
 */
enum nl80211_pmksa_candidate_attr {
	__NL80211_PMKSA_CANDIDATE_INVALID,
	NL80211_PMKSA_CANDIDATE_INDEX,
	NL80211_PMKSA_CANDIDATE_BSSID,
	NL80211_PMKSA_CANDIDATE_PREAUTH,

	/* keep last */
	NUM_NL80211_PMKSA_CANDIDATE,
	MAX_NL80211_PMKSA_CANDIDATE = NUM_NL80211_PMKSA_CANDIDATE - 1
};

/**
 * enum nl80211_tdls_operation - values for %NL80211_ATTR_TDLS_OPERATION
 * @NL80211_TDLS_DISCOVERY_REQ: Send a TDLS discovery request
 * @NL80211_TDLS_SETUP: Setup TDLS link
 * @NL80211_TDLS_TEARDOWN: Teardown a TDLS link which is already established
 * @NL80211_TDLS_ENABLE_LINK: Enable TDLS link
 * @NL80211_TDLS_DISABLE_LINK: Disable TDLS link
 */
enum nl80211_tdls_operation {
	NL80211_TDLS_DISCOVERY_REQ,
	NL80211_TDLS_SETUP,
	NL80211_TDLS_TEARDOWN,
	NL80211_TDLS_ENABLE_LINK,
	NL80211_TDLS_DISABLE_LINK,
};

/*
 * enum nl80211_ap_sme_features - device-integrated AP features
 * Reserved for future use, no bits are defined in
 * NL80211_ATTR_DEVICE_AP_SME yet.
enum nl80211_ap_sme_features {
};
 */

/**
 * enum nl80211_feature_flags - device/driver features
 * @NL80211_FEATURE_SK_TX_STATUS: This driver supports reflecting back
 *	TX status to the socket error queue when requested with the
 *	socket option.
 * @NL80211_FEATURE_HT_IBSS: This driver supports IBSS with HT datarates.
 */
enum nl80211_feature_flags {
	NL80211_FEATURE_SK_TX_STATUS	= 1 << 0,
	NL80211_FEATURE_HT_IBSS		= 1 << 1,
};

/**
 * enum nl80211_probe_resp_offload_support_attr - optional supported
 *	protocols for probe-response offloading by the driver/FW.
 *	To be used with the %NL80211_ATTR_PROBE_RESP_OFFLOAD attribute.
 *	Each enum value represents a bit in the bitmap of supported
 *	protocols. Typically a subset of probe-requests belonging to a
 *	supported protocol will be excluded from offload and uploaded
 *	to the host.
 *
 * @NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS: Support for WPS ver. 1
 * @NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS2: Support for WPS ver. 2
 * @NL80211_PROBE_RESP_OFFLOAD_SUPPORT_P2P: Support for P2P
 * @NL80211_PROBE_RESP_OFFLOAD_SUPPORT_80211U: Support for 802.11u
 */
enum nl80211_probe_resp_offload_support_attr {
	NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS =	1<<0,
	NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS2 =	1<<1,
	NL80211_PROBE_RESP_OFFLOAD_SUPPORT_P2P =	1<<2,
	NL80211_PROBE_RESP_OFFLOAD_SUPPORT_80211U =	1<<3,
};
	
#ifdef USE_QCA_NL80211_SUBCMD
#ifndef QCA_VENDOR_OUI
#define QCA_VENDOR_OUI 0x001374
#endif
enum qca_wlan_genric_data {
	QCA_WLAN_VENDOR_ATTR_PARAM_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_PARAM_DATA,
	QCA_WLAN_VENDOR_ATTR_PARAM_LENGTH,
	QCA_WLAN_VENDOR_ATTR_PARAM_FLAGS,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_PARAM_LAST,
	QCA_WLAN_VENDOR_ATTR_PARAM_MAX =
		QCA_WLAN_VENDOR_ATTR_PARAM_LAST - 1
};


/**
 * enum qca_nl80211_vendor_subcmds: NL 80211 vendor sub command
 *
 * @QCA_NL80211_VENDOR_SUBCMD_UNSPEC: Unspecified
 * @QCA_NL80211_VENDOR_SUBCMD_TEST: Test
 *	Sub commands 2 to 8 are not used
 * @QCA_NL80211_VENDOR_SUBCMD_ROAMING: Roaming
 * @QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY: Avoid frequency.
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_CAPABILITY: DFS capability
 * @QCA_NL80211_VENDOR_SUBCMD_NAN: NAN command/event which is used to pass
 *	NAN Request/Response and NAN Indication messages. These messages are
 *	interpreted between the framework and the firmware component. While
 *	sending the command from userspace to the driver, payload is not
 *	encapsulated inside any attribute. Attribute QCA_WLAN_VENDOR_ATTR_NAN
 *	is used when receiving vendor events in userspace from the driver.
 * @QCA_NL80211_VENDOR_SUBCMD_STATS_EXT: Ext stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET: Link layer stats set
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET: Link layer stats get
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR: Link layer stats clear
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_RADIO_RESULTS: Link layer stats radio
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_IFACE_RESULTS: Link layer stats interface
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_PEERS_RESULTS: Link layer stats peer
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START: Ext scan start
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP: Ext scan stop
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS: Ext scan get valid
 *	channels
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES: Ext scan get capability
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS: Ext scan get cached
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE: Ext scan results
 *	available. Used when report_threshold is reached in scan cache.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT: Ext scan full scan
 *	result. Used to report scan results when each probe rsp. is received,
 *	if report_events enabled in wifi_scan_cmd_params.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT: Ext scan event from target.
 *	Indicates progress of scanning state-machine.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND: Ext scan hotlist
 *	ap found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST: Ext scan set hotlist
 *	bssid
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST: Ext scan reset
 *	hotlist bssid
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE: Ext scan significant
 *	change
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE: Ext scan set
 *	significant change
 *	ap found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE: Ext scan reset
 *	significant change
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_ENABLE: Ext tdls enable
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_DISABLE: Ext tdls disable
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_STATUS: Ext tdls get status
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE: Ext tdls state
 * @QCA_NL80211_VENDOR_SUBCMD_GET_SUPPORTED_FEATURES: Get supported features
 * @QCA_NL80211_VENDOR_SUBCMD_SCANNING_MAC_OUI: Set scanning_mac_oui
 * @QCA_NL80211_VENDOR_SUBCMD_NO_DFS_FLAG: No DFS flag
 * @QCA_NL80211_VENDOR_SUBCMD_GET_CONCURRENCY_MATRIX: Get Concurrency Matrix
 * @QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_SET_KEY: Get the key mgmt offload keys
 * @QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH: After roaming, send the
 * roaming and auth information.
 * @QCA_NL80211_VENDOR_SUBCMD_OCB_SET_SCHED: Set OCB schedule
 *
 * @QCA_NL80211_VENDOR_SUBCMD_DO_ACS: ACS command/event which is used to
 *	invoke the ACS function in device and pass selected channels to
 *	hostapd. Uses enum qca_wlan_vendor_attr_acs_offload attributes.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES: Get the supported features by the
 * driver.
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED: Indicate that driver
 *	started CAC on DFS channel
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED: Indicate that driver
 * 	completed the CAC check on DFS channel
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_ABORTED: Indicate that the CAC
 * 	check was aborted by the driver
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_NOP_FINISHED: Indicate that the
 * 	driver completed NOP
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED: Indicate that the
 * 	driver detected radar signal on the current operating channel
 * @QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_INFO: get wlan driver information
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_START: start wifi logger
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_MEMORY_DUMP: memory dump request
 * @QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET: get logger feature set
 * @QCA_NL80211_VENDOR_SUBCMD_ROAM: roam
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST: extscan set ssid hotlist
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SSID_HOTLIST:
 *	extscan reset ssid hotlist
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND: hotlist ssid found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST: hotlist ssid lost
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_LIST: set pno list
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_PASSPOINT_LIST: set passpoint list
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_RESET_PASSPOINT_LIST:
 *	reset passpoint list
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND: pno network found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND:
 *	passpoint network found
 * @QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION: set wifi config
 * @QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION: get wifi config
 * @QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET: get logging features
 * @QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES: get link properties
 * @QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG: set gateway parameters
 * @QCA_NL80211_VENDOR_SUBCMD_GET_PREFERRED_FREQ_LIST: get preferred channel
	list
 * @QCA_NL80211_VENDOR_SUBCMD_SET_PROBABLE_OPER_CHANNEL: channel hint
 * @QCA_NL80211_VENDOR_SUBCMD_SETBAND: Command to configure the band
 *	to the host driver. This command sets the band through either
 *	the attribute QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE or
 *	QCA_WLAN_VENDOR_ATTR_SETBAND_MASK. QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE
 *	refers enum qca_set_band as unsigned integer values and
 *	QCA_WLAN_VENDOR_ATTR_SETBAND_MASK refers it as 32 bit unsigned BitMask
 *	values. Also, the acceptable values for
 *	QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE are only till QCA_SETBAND_2G. Further
 *	values/bitmask's are valid for QCA_WLAN_VENDOR_ATTR_SETBAND_MASK. The
 *	attribute QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE is deprecated and the
 *	recommendation is to use the QCA_WLAN_VENDOR_ATTR_SETBAND_MASK. If the
 *	implementations configure using both the attributes, the configurations
 *	through QCA_WLAN_VENDOR_ATTR_SETBAND_MASK shall always take the
 *	precedence.
 * @QCA_NL80211_VENDOR_SUBCMD_TRIGGER_SCAN: venodr scan command
 * @QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE: vendor scan complete
 * @QCA_NL80211_VENDOR_SUBCMD_ABORT_SCAN: vendor abort scan
 * @QCA_NL80211_VENDOR_SUBCMD_OTA_TEST: enable OTA test
 * @QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE: set tx power by percentage
 * @QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE_DECR_DB: reduce tx power by DB
 * @QCA_NL80211_VENDOR_SUBCMD_SET_SAP_CONFIG: SAP configuration
 * @QCA_NL80211_VENDOR_SUBCMD_TSF: TSF operations command
 * @QCA_NL80211_VENDOR_SUBCMD_WISA: WISA mode configuration
 * @QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_START: Command used to
 *	start the P2P Listen Offload function in device and pass the listen
 *	channel, period, interval, count, number of device types, device
 *	types and vendor information elements to device driver and firmware.
 * @QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_STOP: Command/event used to
 *	indicate stop request/response of the P2P Listen Offload function in
 *	device. As an event, it indicates either the feature stopped after it
 *	was already running or feature has actually failed to start.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_STATION: send BSS Information
 * @QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH: After SAP starts
 *     beaconing, this sub command provides the driver, the frequencies on the
 *     5 GHz to check for any radar activity. Driver selects one channel from
 *     this priority list provided through
 *     @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_FREQ_LIST and starts
 *     to check for radar activity on it. If no radar activity is detected
 *     during the channel availability check period, driver internally switches
 *     to the selected frequency of operation. If the frequency is zero, driver
 *     internally selects a channel. The status of this conditional switch is
 *     indicated through an event using the same sub command through
 *     @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_STATUS. Attributes are
 *     listed in qca_wlan_vendor_attr_sap_conditional_chan_switch
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT: Command/event used to config
 *      indication period and threshold for MAC layer counters.
 * @QCA_NL80211_VENDOR_SUBCMD_CONFIGURE_TDLS: Configure the TDLS behavior
 *	in the host driver. The different TDLS configurations are defined
 *	by the attributes in enum qca_wlan_vendor_attr_tdls_configuration.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_HE_CAPABILITIES: Get HE related capabilities
 * @QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS:Set the Specific Absorption Rate
 *	(SAR) power limits. A critical regulation for FCC compliance, OEMs
 *	require methods to set SAR limits on TX power of WLAN/WWAN.
 *	enum qca_vendor_attr_sar_limits attributes are used with this command.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS: Vendor command used to get/set
 *      configuration of vendor ACS.
 * @QCA_NL80211_VENDOR_SUBCMD_CHIP_PWRSAVE_FAILURE: Vendor event carrying the
 *      requisite information leading to a power save failure. The information
 *      carried as part of this event is represented by the
 *      enum qca_attr_chip_power_save_failure attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET: Start/Stop the NUD statistics
 *      collection. Uses attributes defined in enum qca_attr_nud_stats_set.
 * @QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET: Get the NUD statistics. These
 *      statistics are represented by the enum qca_attr_nud_stats_get
 *      attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS: Sub-command to fetch
 *      the BSS transition status, whether accept or reject, for a list of
 *      candidate BSSIDs provided by the userspace. This uses the vendor
 *      attributes QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON and
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO. The userspace shall specify
 *      the attributes QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON and an
 *      array of QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID nested in
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO in the request. In the response
 *      the driver shall specify array of
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID and
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_STATUS pairs nested in
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO.
 * @QCA_NL80211_VENDOR_SUBCMD_SET_TRACE_LEVEL: Set the trace level for a
 *      specific QCA module. The trace levels are represented by
 *      enum qca_attr_trace_level attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_BRP_SET_ANT_LIMIT: Set the Beam Refinement
 *      Protocol antenna limit in different modes. See enum
 *      qca_wlan_vendor_attr_brp_ant_limit_mode.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START: Start spectral scan. The scan
 *      parameters are specified by enum qca_wlan_vendor_attr_spectral_scan.
 *      This returns a cookie (%QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_COOKIE)
 *      identifying the operation in success case. In failure cases an
 *      error code (%QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_ERROR_CODE)
 *      describing the reason for the failure is returned.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_STOP: Stop spectral scan. This uses
 *      a cookie (%QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_COOKIE) from
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START to identify the scan to
 *      be stopped.
 * @QCA_NL80211_VENDOR_SUBCMD_ACTIVE_TOS: Set the active Type Of Service on the
 *     specific interface. This can be used to modify some of the low level
 *     scan parameters (off channel dwell time, home channel time) in the
 *     driver/firmware. These parameters are maintained within the host
 *     driver.
 *     This command is valid only when the interface is in the connected
 *     state.
 *     These scan parameters shall be reset by the driver/firmware once
 *     disconnected. The attributes used with this command are defined in
 *     enum qca_wlan_vendor_attr_active_tos.
 * @QCA_NL80211_VENDOR_SUBCMD_HANG: Event indicating to the user space that the
 *      driver has detected an internal failure. This event carries the
 *      information indicating the reason that triggered this detection. The
 *      attributes for this command are defined in
 *      enum qca_wlan_vendor_attr_hang.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CONFIG: Get the current values
 *     of spectral parameters used. The spectral scan parameters are specified
 *     by enum qca_wlan_vendor_attr_spectral_scan.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_DIAG_STATS: Get the debug stats
 *     for spectral scan functionality. The debug stats are specified by
 *     enum qca_wlan_vendor_attr_spectral_diag_stats.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO: Get spectral
 *     scan system capabilities. The capabilities are specified
 *     by enum qca_wlan_vendor_attr_spectral_cap.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_STATUS: Get the current
 *     status of spectral scan. The status values are specified
 *     by enum qca_wlan_vendor_attr_spectral_scan_status.
 * @QCA_NL80211_VENDOR_SUBCMD_PEER_FLUSH_PENDING: Sub-command to flush
 *     peer pending packets. Specify the peer MAC address in
 *     QCA_WLAN_VENDOR_ATTR_PEER_ADDR and the access category of the packets
 *     in QCA_WLAN_VENDOR_ATTR_AC. The attributes are listed
 *     in enum qca_wlan_vendor_attr_flush_pending.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_RROP_INFO: Get vendor specific Representative
 *     RF Operating Parameter (RROP) information. The attributes for this
 *     information are defined in enum qca_wlan_vendor_attr_rrop_info. This is
 *     intended for use by external Auto Channel Selection applications.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS: Get the Specific Absorption Rate
 *	(SAR) power limits. This is a companion to the command
 *	@QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS and is used to retrieve the
 *	settings currently in use. The attributes returned by this command are
 *	defined by enum qca_vendor_attr_sar_limits.
 * @QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO: Provides the current behaviour of
 *      the WLAN hardware MAC's associated with each WLAN netdev interface.
 *      This works both as a query (user space asks the current mode) or event
 *      interface (driver advertizing the current mode to the user space).
 *      Driver does not trigger this event for temporary hardware mode changes.
 *      Mode changes w.r.t Wi-Fi connection updation ( VIZ creation / deletion,
 *      channel change etc ) are updated with this event. Attributes for this
 *      interface are defined in enum qca_wlan_vendor_attr_mac.
 * @QCA_NL80211_VENDOR_SUBCMD_SET_QDEPTH_THRESH: Set MSDU queue depth threshold
 *	per peer per TID. Attributes for this command are define in
 *	enum qca_wlan_set_qdepth_thresh_attr
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION: Sub command to set WiFi
 *	test configuration. Attributes for this command are defined in
 *	enum qca_wlan_vendor_attr_wifi_test_config.
 * @QCA_NL80211_VENDOR_SUBCMD_NAN_EXT: An extendable version of NAN vendor
 *	command. The earlier command for NAN, QCA_NL80211_VENDOR_SUBCMD_NAN,
 *	carried a payload which was a binary blob of data. The command was not
 *	extendable to send more information. The newer version carries the
 *	legacy blob encapsulated within an attribute and can be extended with
 *	additional vendor attributes that can enhance the NAN command
 *	interface.
 * @QCA_NL80211_VENDOR_SUBCMD_PEER_CFR_CAPTURE_CFG: This command is used to
 *	configure parameters per peer to capture Channel Frequency Response
 *	(CFR) and enable Periodic CFR capture. The attributes for this command
 *	are defined in enum qca_wlan_vendor_peer_cfr_capture_attr.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_FW_STATE: Sub command to get firmware state.
 *	The returned firmware state is specified in the attribute
 *	QCA_WLAN_VENDOR_ATTR_FW_STATE.
 * @QCA_NL80211_VENDOR_SUBCMD_PEER_STATS_CACHE_FLUSH: This vendor subcommand
 *	is used by host driver to flush per-peer cached statistics to user space
 *	application. This interface is used as an event from host driver to
 *	user space application. Attributes for this event are specified in
 *	enum qca_wlan_vendor_attr_peer_stats_cache_params.
 *	QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_DATA attribute is expected to be
 *	sent as event from host driver.
 * @QCA_NL80211_VENDOR_SUBCMD_MPTA_HELPER_CONFIG: This sub command is used to
 *	improve the success rate of Zigbee joining network.
 *	Due to PTA master limitation, zigbee joining network success rate is
 *	low while wlan is working. Wlan host driver need to configure some
 *	parameters including Zigbee state and specific WLAN periods to enhance
 *	PTA master. All this parameters are delivered by the NetLink attributes
 *	defined in "enum qca_mpta_helper_vendor_attr".
 * @QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING: This sub command is used to
 *	implement Beacon frame reporting feature.
 *
 *	Userspace can request the driver/firmware to periodically report
 *	received Beacon frames whose BSSID is same as the current connected
 *	BSS's MAC address.
 *
 *	In case the STA seamlessly (without sending disconnect indication to
 *	userspace) roams to a different BSS, Beacon frame reporting will be
 *	automatically enabled for the Beacon frames whose BSSID is same as the
 *	MAC address of the new BSS. Beacon reporting will be stopped when the
 *	STA is disconnected (when the disconnect indication is sent to
 *	userspace) and need to be explicitly enabled by userspace for next
 *	connection.
 *
 *	When a Beacon frame matching configured conditions is received, and if
 *	userspace has requested to send asynchronous beacon reports, the
 *	driver/firmware will encapsulate the details of the Beacon frame in an
 *	event and send it to userspace along with updating the BSS information
 *	in cfg80211 scan cache, otherwise driver will only update the cfg80211
 *	scan cache with the information from the received Beacon frame but
 *	will not send any active report to userspace.
 *
 *	The userspace can request the driver/firmware to stop reporting Beacon
 *	frames. If the driver/firmware is not able to receive Beacon frames
 *	due to other Wi-Fi operations such as off-channel activities, etc.,
 *	the driver/firmware will send a pause event to userspace and stop
 *	reporting Beacon frames. Whether the beacon reporting will be
 *	automatically resumed or not by the driver/firmware later will be
 *	reported to userspace using the
 *	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_AUTO_RESUMES flag. The beacon
 *	reporting shall be resumed for all the cases except either when
 *	userspace sets QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_DO_NOT_RESUME flag
 *	in the command which triggered the current beacon reporting or during
 *	any disconnection case as indicated by setting
 *	QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_PAUSE_REASON to
 *	QCA_WLAN_VENDOR_BEACON_REPORTING_PAUSE_REASON_DISCONNECTED by the
 *	driver.
 *
 *	After QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_PAUSE event is received
 *	by userspace with QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_AUTO_RESUMES
 *	flag not set, the next first
 *	QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO event from the driver
 *	shall be considered as un-pause event.
 *
 *	All the attributes used with this command are defined in
 *	enum qca_wlan_vendor_attr_beacon_reporting_params.
 * @QCA_NL80211_VENDOR_SUBCMD_INTEROP_ISSUES_AP: In practice, some aps have
 *	interop issues with the DUT. This sub command is used to transfer the
 *	ap info between driver and user space. This works both as a command
 *	or event. As a command, it configs the stored list of aps from user
 *	space to firmware; as an event, it indicates the ap info detected by
 *	firmware to user space for persistent storage. The attributes defined
 *	in enum qca_vendor_attr_interop_issues_ap are used to deliver the
 *	parameters.
 * @QCA_NL80211_VENDOR_SUBCMD_OEM_DATA: This command is used to send OEM data
 *	binary blobs from application/service to firmware. The attributes
 *	defined in enum qca_wlan_vendor_attr_oem_data_params are used to
 *	deliver the parameters.
 * @QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_EXT: This command/event is used
 *	to send/receive avoid frequency data using
 *	enum qca_wlan_vendor_attr_avoid_frequency_ext.
 *	This new command is alternative to existing command
 *	QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY since existing command/event
 *	is using stream of bytes instead of structured data using vendor
 *	attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_ADD_STA_NODE: This vendor subcommand is used to
 *	add the STA node details in driver/firmware. Attributes for this event
 *	are specified in enum qca_wlan_vendor_attr_add_sta_node_params.
 * @QCA_NL80211_VENDOR_SUBCMD_BTC_CHAIN_MODE: This command is used to set BT
 *	coex chain mode from application/service.
 *	The attributes defined in enum qca_vendor_attr_btc_chain_mode are used
 *	to deliver the parameters.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_STA_INFO: This vendor subcommand is used to
 *	get information of a station from driver to userspace. This command can
 *	be used in both STA and AP modes. For STA mode, it provides information
 *	of the current association when in connected state or the last
 *	association when in disconnected state. For AP mode, only information
 *	of the currently connected stations is available. This command uses
 *	attributes defined in enum qca_wlan_vendor_attr_get_sta_info.
 * @QCA_NL80211_VENDOR_SUBCMD_REQUEST_SAR_LIMITS_EVENT: This acts as an event.
 *	Host drivers can request the user space entity to set the SAR power
 *	limits with this event. Accordingly, the user space entity is expected
 *	to set the SAR power limits. Host drivers can retry this event to the
 *	user space for the SAR power limits configuration from user space. If
 *	the driver does not get the SAR power limits from user space for all
 *	the retried attempts, it can configure a default SAR power limit.
 * @QCA_NL80211_VENDOR_SUBCMD_UPDATE_STA_INFO: This acts as a vendor event and
 *	is used to update the information about the station from the driver to
 *	userspace. Uses attributes from enum
 *	qca_wlan_vendor_attr_update_sta_info.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_DRIVER_DISCONNECT_REASON: This acts as an event.
 *	The host driver initiates the disconnection for scenarios such as beacon
 *	miss, NUD failure, peer kick out, etc. The disconnection indication
 *	through cfg80211_disconnected() expects the reason codes from enum
 *	ieee80211_reasoncode which does not signify these various reasons why
 *	the driver has triggered the disconnection. This event will be used to
 *	send the driver specific reason codes by the host driver to userspace.
 *	Host drivers should trigger this event and pass the respective reason
 *	code immediately prior to triggering cfg80211_disconnected(). The
 *	attributes used with this event are defined in enum
 *	qca_wlan_vendor_attr_driver_disconnect_reason.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_CONFIG_TSPEC: This vendor subcommand is used to
 *	add/delete TSPEC for each AC. One command is for one specific AC only.
 *	This command can only be used in STA mode and the STA must be
 *	associated with an AP when the command is issued. Uses attributes
 *	defined in enum qca_wlan_vendor_attr_config_tspec.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT: Vendor subcommand to configure TWT.
 *	Uses attributes defined in enum qca_wlan_vendor_attr_config_twt.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_GETBAND: Command to get the configured band from
 *	the host driver. The band configurations obtained are referred through
 *	QCA_WLAN_VENDOR_ATTR_SETBAND_MASK.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_FW_STATS: This vendor subcommand is used by
 *	the driver to send opaque data from the firmware to userspace. The
 *	driver sends an event to userspace whenever such data is received from
 *	the firmware.
 *
 *	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA is used as the attribute to
 *	send this opaque data for this event.
 *
 *	The format of the opaque data is specific to the particular firmware
 *	version and there is no guarantee of the format remaining same.
 *
 * @QCA_NL80211_VENDOR_SUBCMD_MBSSID_TX_VDEV_STATUS: This acts as an event.
 *	The host driver selects Tx VDEV, and notifies user. The attributes
 *	used with this event are defined in enum
 *	qca_wlan_vendor_attr_mbssid_tx_vdev_status.
 */

enum qca_nl80211_vendor_subcmds {
	QCA_NL80211_VENDOR_SUBCMD_UNSPEC = 0,
	QCA_NL80211_VENDOR_SUBCMD_TEST = 1,
	QCA_NL80211_VENDOR_SUBCMD_ROAMING = 9,
	QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY = 10,
	QCA_NL80211_VENDOR_SUBCMD_DFS_CAPABILITY = 11,
	QCA_NL80211_VENDOR_SUBCMD_NAN = 12,
	QCA_NL80211_VENDOR_SUBCMD_STATS_EXT = 13,

	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET = 14,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET = 15,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR = 16,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_RADIO_RESULTS = 17,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_IFACE_RESULTS = 18,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_PEERS_RESULTS = 19,

	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START = 20,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP = 21,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS = 22,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES = 23,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS = 24,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE = 25,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT = 26,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT = 27,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND = 28,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST = 29,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST = 30,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE = 31,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE = 32,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE = 33,

	QCA_NL80211_VENDOR_SUBCMD_TDLS_ENABLE = 34,
	QCA_NL80211_VENDOR_SUBCMD_TDLS_DISABLE = 35,
	QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_STATUS = 36,
	QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE = 37,

	QCA_NL80211_VENDOR_SUBCMD_GET_SUPPORTED_FEATURES = 38,

	QCA_NL80211_VENDOR_SUBCMD_SCANNING_MAC_OUI = 39,
	QCA_NL80211_VENDOR_SUBCMD_NO_DFS_FLAG = 40,

	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_LOST = 41,

	/* Get Concurrency Matrix */
	QCA_NL80211_VENDOR_SUBCMD_GET_CONCURRENCY_MATRIX = 42,

	QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_SET_KEY = 50,
	QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH = 51,
	QCA_NL80211_VENDOR_SUBCMD_APFIND = 52,

	/* Deprecated */
	QCA_NL80211_VENDOR_SUBCMD_OCB_SET_SCHED = 53,

	QCA_NL80211_VENDOR_SUBCMD_DO_ACS = 54,

	QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES = 55,

	/* Off loaded DFS events */
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED = 56,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED = 57,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_ABORTED = 58,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_NOP_FINISHED = 59,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED = 60,

	QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_INFO = 61,
	QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_START = 62,
	QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_MEMORY_DUMP = 63,
	QCA_NL80211_VENDOR_SUBCMD_ROAM = 64,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST = 65,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SSID_HOTLIST = 66,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND = 67,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST = 68,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_LIST = 69,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_PASSPOINT_LIST = 70,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_RESET_PASSPOINT_LIST = 71,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND = 72,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND = 73,

	/* Wi-Fi Configuration subcommands */
	QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION = 74,
	QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION = 75,
	QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET = 76,
	QCA_NL80211_VENDOR_SUBCMD_GET_RING_DATA = 77,

	QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_CAPABILITIES = 78,
	QCA_NL80211_VENDOR_SUBCMD_OFFLOADED_PACKETS = 79,
	QCA_NL80211_VENDOR_SUBCMD_MONITOR_RSSI = 80,
	QCA_NL80211_VENDOR_SUBCMD_NDP = 81,

	/* NS Offload enable/disable cmd */
	QCA_NL80211_VENDOR_SUBCMD_ND_OFFLOAD = 82,

	QCA_NL80211_VENDOR_SUBCMD_PACKET_FILTER = 83,
	QCA_NL80211_VENDOR_SUBCMD_GET_BUS_SIZE = 84,

	QCA_NL80211_VENDOR_SUBCMD_GET_WAKE_REASON_STATS = 85,

	QCA_NL80211_VENDOR_SUBCMD_DATA_OFFLOAD = 91,
	/* OCB commands */
	QCA_NL80211_VENDOR_SUBCMD_OCB_SET_CONFIG = 92,
	QCA_NL80211_VENDOR_SUBCMD_OCB_SET_UTC_TIME = 93,
	QCA_NL80211_VENDOR_SUBCMD_OCB_START_TIMING_ADVERT = 94,
	QCA_NL80211_VENDOR_SUBCMD_OCB_STOP_TIMING_ADVERT = 95,
	QCA_NL80211_VENDOR_SUBCMD_OCB_GET_TSF_TIMER = 96,
	QCA_NL80211_VENDOR_SUBCMD_DCC_GET_STATS = 97,
	QCA_NL80211_VENDOR_SUBCMD_DCC_CLEAR_STATS = 98,
	QCA_NL80211_VENDOR_SUBCMD_DCC_UPDATE_NDL = 99,
	QCA_NL80211_VENDOR_SUBCMD_DCC_STATS_EVENT = 100,

	/* subcommand to get link properties */
	QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES = 101,
	/* LFR Subnet Detection */
	QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG = 102,

	/* DBS subcommands */
	QCA_NL80211_VENDOR_SUBCMD_GET_PREFERRED_FREQ_LIST = 103,
	QCA_NL80211_VENDOR_SUBCMD_SET_PROBABLE_OPER_CHANNEL = 104,

	/* Vendor setband command */
	QCA_NL80211_VENDOR_SUBCMD_SETBAND = 105,

	/* Vendor scan commands */
	QCA_NL80211_VENDOR_SUBCMD_TRIGGER_SCAN = 106,
	QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE = 107,

	/* OTA test subcommand */
	QCA_NL80211_VENDOR_SUBCMD_OTA_TEST = 108,
	/* Tx power scaling subcommands */
	QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE = 109,
	/* Tx power scaling in db subcommands */
	QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE_DECR_DB = 115,
	QCA_NL80211_VENDOR_SUBCMD_ACS_POLICY = 116,
	QCA_NL80211_VENDOR_SUBCMD_STA_CONNECT_ROAM_POLICY = 117,
	QCA_NL80211_VENDOR_SUBCMD_SET_SAP_CONFIG  = 118,
	QCA_NL80211_VENDOR_SUBCMD_TSF = 119,
	QCA_NL80211_VENDOR_SUBCMD_WISA = 120,
	QCA_NL80211_VENDOR_SUBCMD_GET_STATION = 121,
	QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_START = 122,
	QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_STOP = 123,
	QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH = 124,
	QCA_NL80211_VENDOR_SUBCMD_GPIO_CONFIG_COMMAND = 125,

	QCA_NL80211_VENDOR_SUBCMD_GET_HW_CAPABILITY = 126,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT = 127,
	/* FTM/indoor location subcommands */
	QCA_NL80211_VENDOR_SUBCMD_LOC_GET_CAPA = 128,
	QCA_NL80211_VENDOR_SUBCMD_FTM_START_SESSION = 129,
	QCA_NL80211_VENDOR_SUBCMD_FTM_ABORT_SESSION = 130,
	QCA_NL80211_VENDOR_SUBCMD_FTM_MEAS_RESULT = 131,
	QCA_NL80211_VENDOR_SUBCMD_FTM_SESSION_DONE = 132,
	QCA_NL80211_VENDOR_SUBCMD_FTM_CFG_RESPONDER = 133,
	QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS = 134,
	QCA_NL80211_VENDOR_SUBCMD_AOA_ABORT_MEAS = 135,
	QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS_RESULT = 136,

	/* Encrypt/Decrypt command */
	QCA_NL80211_VENDOR_SUBCMD_ENCRYPTION_TEST = 137,

	QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI = 138,
	/* DMG low level RF sector operations */
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_GET_SECTOR_CFG = 139,
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_SET_SECTOR_CFG = 140,
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_GET_SELECTED_SECTOR = 141,
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_SET_SELECTED_SECTOR = 142,

	/* Configure the TDLS mode from user space */
	QCA_NL80211_VENDOR_SUBCMD_CONFIGURE_TDLS = 143,

	QCA_NL80211_VENDOR_SUBCMD_GET_HE_CAPABILITIES = 144,

	/* Vendor abort scan command */
	QCA_NL80211_VENDOR_SUBCMD_ABORT_SCAN = 145,

	/* Set Specific Absorption Rate(SAR) Power Limits */
	QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS = 146,

	/* External Auto channel configuration setting */
	QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS = 147,

	QCA_NL80211_VENDOR_SUBCMD_CHIP_PWRSAVE_FAILURE = 148,
	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET = 149,
	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET = 150,
	QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS = 151,

	/* Set the trace level for QDF */
	QCA_NL80211_VENDOR_SUBCMD_SET_TRACE_LEVEL = 152,

	QCA_NL80211_VENDOR_SUBCMD_BRP_SET_ANT_LIMIT = 153,

	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START = 154,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_STOP = 155,
	QCA_NL80211_VENDOR_SUBCMD_ACTIVE_TOS = 156,
	QCA_NL80211_VENDOR_SUBCMD_HANG = 157,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CONFIG = 158,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_DIAG_STATS = 159,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO = 160,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_STATUS = 161,
	QCA_NL80211_VENDOR_SUBCMD_PEER_FLUSH_PENDING = 162,
	QCA_NL80211_VENDOR_SUBCMD_GET_RROP_INFO = 163,
	QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS = 164,
	QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO = 165,
	QCA_NL80211_VENDOR_SUBCMD_SET_QDEPTH_THRESH = 166,
	QCA_NL80211_VENDOR_SUBCMD_THERMAL_CMD = 167,
	/* Wi-Fi test configuration subcommand */
	QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION = 169,
	QCA_NL80211_VENDOR_SUBCMD_NAN_EXT = 171,
	QCA_NL80211_VENDOR_SUBCMD_PEER_CFR_CAPTURE_CFG = 173,
	QCA_NL80211_VENDOR_SUBCMD_THROUGHPUT_CHANGE_EVENT = 174,
	QCA_NL80211_VENDOR_SUBCMD_COEX_CONFIG = 175,
	QCA_NL80211_VENDOR_SUBCMD_GET_FW_STATE = 177,
	QCA_NL80211_VENDOR_SUBCMD_PEER_STATS_CACHE_FLUSH = 178,
	QCA_NL80211_VENDOR_SUBCMD_MPTA_HELPER_CONFIG = 179,
	QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING = 180,
	QCA_NL80211_VENDOR_SUBCMD_INTEROP_ISSUES_AP = 181,
	QCA_NL80211_VENDOR_SUBCMD_OEM_DATA = 182,
	QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_EXT = 183,
	QCA_NL80211_VENDOR_SUBCMD_ADD_STA_NODE = 184,
	QCA_NL80211_VENDOR_SUBCMD_BTC_CHAIN_MODE = 185,
	QCA_NL80211_VENDOR_SUBCMD_GET_STA_INFO = 186,
	QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS_EVENT = 187,
	QCA_NL80211_VENDOR_SUBCMD_UPDATE_STA_INFO = 188,
	QCA_NL80211_VENDOR_SUBCMD_DRIVER_DISCONNECT_REASON = 189,
	QCA_NL80211_VENDOR_SUBCMD_CONFIG_TSPEC = 190,
	QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT = 191,
	QCA_NL80211_VENDOR_SUBCMD_GETBAND = 192,
	QCA_NL80211_VENDOR_SUBCMD_WIFI_FW_STATS = 195,
	QCA_NL80211_VENDOR_SUBCMD_MBSSID_TX_VDEV_STATUS = 196,
};

enum qca_wlan_vendor_attr_config {
	QCA_WLAN_VENDOR_ATTR_CONFIG_INVALID = 0,
	/*
	 * Unsigned 32-bit value to set the DTIM period.
	 * Whether the wifi chipset wakes at every dtim beacon or a multiple of
	 * the DTIM period. If DTIM is set to 3, the STA shall wake up every 3
	 * DTIM beacons.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_MODULATED_DTIM = 1,
	/*
	 * Unsigned 32-bit value to set the wifi_iface stats averaging factor
	 * used to calculate statistics like average the TSF offset or average
	 * number of frame leaked.
	 * For instance, upon Beacon frame reception:
	 * current_avg = ((beacon_TSF - TBTT) * factor + previous_avg * (0x10000 - factor) ) / 0x10000
	 * For instance, when evaluating leaky APs:
	 * current_avg = ((num frame received within guard time) * factor + previous_avg * (0x10000 - factor)) / 0x10000
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_STATS_AVG_FACTOR = 2,
	/*
	 * Unsigned 32-bit value to configure guard time, i.e., when
	 * implementing IEEE power management based on frame control PM bit, how
	 * long the driver waits before shutting down the radio and after
	 * receiving an ACK frame for a Data frame with PM bit set.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GUARD_TIME = 3,
	/* Unsigned 32-bit value to change the FTM capability dynamically */
	QCA_WLAN_VENDOR_ATTR_CONFIG_FINE_TIME_MEASUREMENT = 4,
	/* Unsigned 16-bit value to configure maximum TX rate dynamically */
	QCA_WLAN_VENDOR_ATTR_CONF_TX_RATE = 5,
	/*
	 * Unsigned 32-bit value to configure the number of continuous
	 * Beacon Miss which shall be used by the firmware to penalize
	 * the RSSI.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PENALIZE_AFTER_NCONS_BEACON_MISS = 6,
	/*
	 * Unsigned 8-bit value to configure the channel avoidance indication
	 * behavior. Firmware to send only one indication and ignore duplicate
	 * indications when set to avoid multiple Apps wakeups.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_AVOIDANCE_IND = 7,
	/*
	 * 8-bit unsigned value to configure the maximum TX MPDU for
	 * aggregation.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MPDU_AGGREGATION = 8,
	/*
	 * 8-bit unsigned value to configure the maximum RX MPDU for
	 * aggregation.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MPDU_AGGREGATION = 9,
	/*
	 * 8-bit unsigned value to configure the Non aggregrate/11g sw
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_NON_AGG_RETRY = 10,
	/*
	 * 8-bit unsigned value to configure the aggregrate sw
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_AGG_RETRY = 11,
	/*
	 * 8-bit unsigned value to configure the MGMT frame
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_MGMT_RETRY = 12,
	/*
	 * 8-bit unsigned value to configure the CTRL frame
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_CTRL_RETRY = 13,
	/*
	 * 8-bit unsigned value to configure the propagation delay for
	 * 2G/5G band (0~63, units in us)
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_DELAY = 14,
	/*
	 * Unsigned 32-bit value to configure the number of unicast TX fail
	 * packet count. The peer is disconnected once this threshold is
	 * reached.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_FAIL_COUNT = 15,
	/*
	 * Attribute used to set scan default IEs to the driver.
	 *
	 * These IEs can be used by scan operations that will be initiated by
	 * the driver/firmware.
	 *
	 * For further scan requests coming to the driver, these IEs should be
	 * merged with the IEs received along with scan request coming to the
	 * driver. If a particular IE is present in the scan default IEs but not
	 * present in the scan request, then that IE should be added to the IEs
	 * sent in the Probe Request frames for that scan request.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_DEFAULT_IES = 16,
	/* Unsigned 32-bit attribute for generic commands */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_COMMAND = 17,
	/* Unsigned 32-bit value attribute for generic commands */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_VALUE = 18,
	/* Unsigned 32-bit data attribute for generic command response */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA = 19,
	/*
	 * Unsigned 32-bit length attribute for
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_LENGTH = 20,
	/*
	 * Unsigned 32-bit flags attribute for
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_FLAGS = 21,
	/*
	 * Unsigned 32-bit, defining the access policy.
	 * See enum qca_access_policy. Used with
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY = 22,
	/*
	 * Sets the list of full set of IEs for which a specific access policy
	 * has to be applied. Used along with
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY to control the access.
	 * Zero length payload can be used to clear this access constraint.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST = 23,
	/*
	 * Unsigned 32-bit, specifies the interface index (netdev) for which the
	 * corresponding configurations are applied. If the interface index is
	 * not specified, the configurations are attributed to the respective
	 * wiphy.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_IFINDEX = 24,
	/*
	 * 8-bit unsigned value to trigger QPower:
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_QPOWER = 25,
	/*
	 * 8-bit unsigned value to configure the driver and below layers to
	 * ignore the assoc disallowed set by APs while connecting
	 * 1-Ignore, 0-Don't ignore
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_IGNORE_ASSOC_DISALLOWED = 26,
	/*
	 * 32-bit unsigned value to trigger antenna diversity features:
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ENA = 27,
	/* 32-bit unsigned value to configure specific chain antenna */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_CHAIN = 28,
	/*
	 * 32-bit unsigned value to trigger cycle selftest
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST = 29,
	/*
	 * 32-bit unsigned to configure the cycle time of selftest
	 * the unit is micro-second
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST_INTVL = 30,
	/* 32-bit unsigned value to set reorder timeout for AC_VO */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_VOICE = 31,
	/* 32-bit unsigned value to set reorder timeout for AC_VI */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_VIDEO = 32,
	/* 32-bit unsigned value to set reorder timeout for AC_BE */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_BESTEFFORT = 33,
	/* 32-bit unsigned value to set reorder timeout for AC_BK */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_BACKGROUND = 34,
	/* 6-byte MAC address to point out the specific peer */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_BLOCKSIZE_PEER_MAC = 35,
	/* 32-bit unsigned value to set window size for specific peer */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_BLOCKSIZE_WINLIMIT = 36,
	/* 8-bit unsigned value to set the beacon miss threshold in 2.4 GHz */
	QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_MISS_THRESHOLD_24 = 37,
	/* 8-bit unsigned value to set the beacon miss threshold in 5 GHz */
	QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_MISS_THRESHOLD_5 = 38,
	/*
	 * 32-bit unsigned value to configure 5 or 10 MHz channel width for
	 * station device while in disconnect state. The attribute use the
	 * value of enum nl80211_chan_width: NL80211_CHAN_WIDTH_5 means 5 MHz,
	 * NL80211_CHAN_WIDTH_10 means 10 MHz. If set, the device work in 5 or
	 * 10 MHz channel width, the station will not connect to a BSS using 20
	 * MHz or higher bandwidth. Set to NL80211_CHAN_WIDTH_20_NOHT to
	 * clear this constraint.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_SUB20_CHAN_WIDTH = 39,
	/*
	 * 32-bit unsigned value to configure the propagation absolute delay
	 * for 2G/5G band (units in us)
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_ABS_DELAY = 40,
	/* 32-bit unsigned value to set probe period */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_PERIOD = 41,
	/* 32-bit unsigned value to set stay period */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_STAY_PERIOD = 42,
	/* 32-bit unsigned value to set snr diff */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SNR_DIFF = 43,
	/* 32-bit unsigned value to set probe dwell time */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_DWELL_TIME = 44,
	/* 32-bit unsigned value to set mgmt snr weight */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_MGMT_SNR_WEIGHT = 45,
	/* 32-bit unsigned value to set data snr weight */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_DATA_SNR_WEIGHT = 46,
	/* 32-bit unsigned value to set ack snr weight */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ACK_SNR_WEIGHT = 47,
	/*
	 * 32-bit unsigned value to configure the listen interval.
	 * This is in units of beacon intervals. This configuration alters
	 * the negotiated listen interval with the AP during the connection.
	 * It is highly recommended to configure a value less than or equal to
	 * the one negotiated during the association. Configuring any greater
	 * value can have adverse effects (frame loss, AP disassociating STA,
	 * etc.).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LISTEN_INTERVAL = 48,
	/*
	 * 8 bit unsigned value that is set on an AP/GO virtual interface to
	 * disable operations that would cause the AP/GO to leave its operating
	 * channel.
	 *
	 * This will restrict the scans to the AP/GO operating channel and the
	 * channels of the other band, if DBS is supported.A STA/CLI interface
	 * brought up after this setting is enabled, will be restricted to
	 * connecting to devices only on the AP/GO interface's operating channel
	 * or on the other band in DBS case. P2P supported channel list is
	 * modified, to only include AP interface's operating-channel and the
	 * channels of the other band if DBS is supported.
	 *
	 * These restrictions are only applicable as long as the AP/GO interface
	 * is alive. If the AP/GO interface is brought down then this
	 * setting/restriction is forgotten.
	 *
	 * If this variable is set on an AP/GO interface while a multi-channel
	 * concurrent session is active, it has no effect on the operation of
	 * the current interfaces, other than restricting the scan to the AP/GO
	 * operating channel and the other band channels if DBS is supported.
	 * However, if the STA is brought down and restarted then the new STA
	 * connection will either be formed on the AP/GO channel or on the
	 * other band in a DBS case. This is because of the scan being
	 * restricted on these channels as mentioned above.
	 *
	 * 1-Disable offchannel operations, 0-Enable offchannel operations.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RESTRICT_OFFCHANNEL = 49,

	/*
	 * 8 bit unsigned value to enable/disable LRO (Large Receive Offload)
	 * on an interface.
	 * 1 - Enable , 0 - Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LRO = 50,

	/*
	 * 8 bit unsigned value to globally enable/disable scan
	 * 1 - Enable, 0 - Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_ENABLE = 51,

	/* 8-bit unsigned value to set the total beacon miss count */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TOTAL_BEACON_MISS_COUNT = 52,

	/*
	 * Unsigned 32-bit value to configure the number of continuous
	 * Beacon Miss which shall be used by the firmware to penalize
	 * the RSSI for BTC.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PENALIZE_AFTER_NCONS_BEACON_MISS_BTC = 53,

	/*
	 * 8-bit unsigned value to configure the driver and below layers to
	 * enable/disable all fils features.
	 * 0-enable, 1-disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_DISABLE_FILS = 54,

	/* 16-bit unsigned value to configure the level of WLAN latency
	 * module. See enum qca_wlan_vendor_attr_config_latency_level.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL = 55,

	/*
	 * 8-bit unsigned value indicating the driver to use the RSNE as-is from
	 * the connect interface. Exclusively used for the scenarios where the
	 * device is used as a test bed device with special functionality and
	 * not recommended for production. This helps driver to not validate the
	 * RSNE passed from user space and thus allow arbitrary IE data to be
	 * used for testing purposes.
	 * 1-enable, 0-disable.
	 * Applications set/reset this configuration. If not reset, this
	 * parameter remains in use until the driver is unloaded.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE = 56,

	/*
	 * 8-bit unsigned value to trigger green Tx power saving.
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GTX = 57,

	/*
	 * Attribute to configure disconnect IEs to the driver.
	 * This carries an array of unsigned 8-bit characters.
	 *
	 * If this is configured, driver shall fill the IEs in disassoc/deauth
	 * frame.
	 * These IEs are expected to be considered only for the next
	 * immediate disconnection (disassoc/deauth frame) originated by
	 * the DUT, irrespective of the entity (user space/driver/firmware)
	 * triggering the disconnection.
	 * The host drivers are not expected to use the IEs set through
	 * this interface for further disconnections after the first immediate
	 * disconnection initiated post the configuration.
	 * If the IEs are also updated through cfg80211 interface (after the
	 * enhancement to cfg80211_disconnect), host driver is expected to
	 * take the union of IEs from both of these interfaces and send in
	 * further disassoc/deauth frames.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_DISCONNECT_IES = 58,

	/* 8-bit unsigned value for ELNA bypass.
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ELNA_BYPASS = 59,

	/* 8-bit unsigned value. This attribute enables/disables the host driver
	 * to send the Beacon Report response with failure reason for the
	 * scenarios where STA cannot honor the Beacon report request from AP.
	 * 1-Enable, 0-Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_REPORT_FAIL = 60,

	/* 8-bit unsigned value. This attribute enables/disables the host driver
	 * to send roam reason information in the reassociation request to the
	 * AP. 1-Enable, 0-Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ROAM_REASON = 61,

	/* 32-bit unsigned value to configure different PHY modes to the
	 * driver/firmware. The possible values are defined in
	 * enum qca_wlan_vendor_phy_mode. The configuration will be reset to
	 * default value, i.e., QCA_WLAN_VENDOR_PHY_MODE_AUTO upon restarting
	 * the driver.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PHY_MODE = 62,

	/* 8-bit unsigned value to configure the maximum supported channel width
	 * for STA mode. If this value is configured when STA is in connected
	 * state, it should not exceed the negotiated channel width. If it is
	 * configured when STA is in disconnected state, the configured value
	 * will take effect for the next immediate connection.
	 * Possible values are:
	 *   NL80211_CHAN_WIDTH_20
	 *   NL80211_CHAN_WIDTH_40
	 *   NL80211_CHAN_WIDTH_80
	 *   NL80211_CHAN_WIDTH_80P80
	 *   NL80211_CHAN_WIDTH_160
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_WIDTH = 63,

	/* 8-bit unsigned value to enable/disable dynamic bandwidth adjustment.
	 * This attribute is only applicable for STA mode. When dynamic
	 * bandwidth adjustment is disabled, STA will use static channel width
	 * the value of which is negotiated during connection.
	 * 1-enable (default), 0-disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_DYNAMIC_BW = 64,

	/* 8-bit unsigned value to configure the maximum number of subframes of
	 * TX MSDU for aggregation. Possible values are 0-31. When set to 0,
	 * it is decided by hardware.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MSDU_AGGREGATION = 65,

	/* 8-bit unsigned value to configure the maximum number of subframes of
	 * RX MSDU for aggregation. Possible values are 0-31. When set to 0,
	 * it is decided by hardware.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MSDU_AGGREGATION = 66,

	/* 8-bit unsigned value. This attribute is used to dynamically
	 * enable/disable the LDPC capability of the device. When configured in
	 * the disconnected state, the updated configuration will be considered
	 * for the immediately following connection attempt. If this
	 * configuration is modified while the device is in the connected state,
	 * the LDPC TX will be updated with this configuration immediately,
	 * while the LDPC RX configuration update will take place starting from
	 * the subsequent association attempt.
	 * 1-Enable, 0-Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LDPC = 67,

	/* 8-bit unsigned value. This attribute is used to dynamically
	 * enable/disable the TX STBC capability of the device. When configured
	 * in the disconnected state, the updated configuration will be
	 * considered for the immediately following connection attempt. If the
	 * connection is formed with TX STBC enabled and if this configuration
	 * is disabled during that association, the TX will be impacted
	 * immediately. Further connection attempts will disable TX STBC.
	 * However, enabling the TX STBC for a connected session with disabled
	 * capability is not allowed and will fail.
	 * 1-Enable, 0-Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_STBC = 68,

	/* 8-bit unsigned value. This attribute is used to dynamically
	 * enable/disable the RX STBC capability of the device. When configured
	 * in the disconnected state, the updated configuration will be
	 * considered for the immediately following connection attempt. If the
	 * configuration is modified in the connected state, there will be no
	 * impact for the current association, but further connection attempts
	 * will use the updated configuration.
	 * 1-Enable, 0-Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_STBC = 69,

	/* 8-bit unsigned value. This attribute is used to dynamically configure
	 * the number of spatial streams. When configured in the disconnected
	 * state, the updated configuration will be considered for the
	 * immediately following connection attempt. If the NSS is updated after
	 * the connection, the updated NSS value is notified to the peer using
	 * the Operating Mode Notification/Spatial Multiplexing Power Save
	 * frame. The updated NSS value after the connection shall not be
	 * greater than the one negotiated during the connection. Any such
	 * higher value configuration shall be returned with a failure.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_NSS = 70,

	/*
	 * 8-bit unsigned value to trigger Optimized Power Management:
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_OPTIMIZED_POWER_MANAGEMENT = 71,

	/* 8-bit unsigned value. This attribute takes the QOS/access category
	 * value represented by the enum qca_wlan_ac_type and expects the driver
	 * to upgrade the UDP frames to this QOS. The value of QCA_WLAN_AC_ALL
	 * is invalid for this attribute. This will override the DSCP value
	 * configured in the frame with the intention to only upgrade the QOS.
	 * That said, it is not intended to downgrade the QOS for the frames.
	 * Set the value to 0 ( corresponding to BE ) if the QOS upgrade needs
	 * to disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_UDP_QOS_UPGRADE = 72,

	/* 8-bit unsigned value. This attribute is used to dynamically configure
	 * the number of chains to be used for transmitting data. This
	 * configuration is allowed only when in connected state and will be
	 * effective until disconnected. The driver rejects this configuration
	 * if the number of spatial streams being used in the current connection
	 * cannot be supported by this configuration.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_NUM_TX_CHAINS = 73,

	/* 8-bit unsigned value. This attribute is used to dynamically configure
	 * the number of chains to be used for receiving data. This
	 * configuration is allowed only when in connected state and will be
	 * effective until disconnected. The driver rejects this configuration
	 * if the number of spatial streams being used in the current connection
	 * cannot be supported by this configuration.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_NUM_RX_CHAINS = 74,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_CONFIG_MAX =
	QCA_WLAN_VENDOR_ATTR_CONFIG_AFTER_LAST - 1,
};

enum qca_nl80211_vendor_subcmds_internal {
    QCA_NL80211_VENDOR_SUBCMD_WIFI_PARAMS = 200,
    QCA_NL80211_VENDOR_SUBCMD_GET_PARAMS = 201,
    QCA_NL80211_VENDOR_SUBCMD_NAWDS_PARAMS = 203,
    QCA_NL80211_VENDOR_SUBCMD_HMWDS_PARAMS = 204,
    QCA_NL80211_VENDOR_SUBCMD_ALD_PARAMS = 205,
    QCA_NL80211_VENDOR_SUBCMD_ATF = 206,
    QCA_NL80211_VENDOR_SUBCMD_WNM = 207,
    QCA_NL80211_VENDOR_SUBCMD_ME_LIST = 208,
    QCA_NL80211_VENDOR_SUBCMD_SET_MAXRATE = 209,
    QCA_NL80211_VENDOR_SUBCMD_VENDORIE = 210,
    QCA_NL80211_VENDOR_SUBCMD_NAC = 211,
    QCA_NL80211_VENDOR_SUBCMD_LIST_SCAN = 212,
    QCA_NL80211_VENDOR_SUBCMD_LIST_CAP = 213,
    QCA_NL80211_VENDOR_SUBCMD_LIST_STA = 214,
    QCA_NL80211_VENDOR_SUBCMD_ACTIVECHANLIST = 215,
    QCA_NL80211_VENDOR_SUBCMD_LIST_CHAN = 216,
    QCA_NL80211_VENDOR_SUBCMD_LIST_CHAN160 = 217,
    QCA_NL80211_VENDOR_SUBCMD_DBGREQ     = 218,
    QCA_NL80211_VENDOR_SUBCMD_PHYSTATS = 219,
    QCA_NL80211_VENDOR_SUBCMD_ATHSTATS = 220,
    QCA_NL80211_VENDOR_SUBCMD_PHYSTATSCUR = 221,
    QCA_NL80211_VENDOR_SUBCMD_EXTENDEDSTATS = 222,
    QCA_NL80211_VENDOR_SUBCMD_80211STATS = 223,
    QCA_NL80211_VENDOR_SUBCMD_STA_STATS = 224,
    QCA_NL80211_VENDOR_SUBCMD_CLONEPARAMS = 225,

    QCA_NL80211_VENDORSUBCMD_ADDMAC = 226,
    QCA_NL80211_VENDORSUBCMD_DELMAC = 227,
    QCA_NL80211_VENDORSUBCMD_KICKMAC = 228,
    QCA_NL80211_VENDORSUBCMD_CHAN_SWITCH = 229,
    QCA_NL80211_VENDORSUBCMD_WIRELESS_MODE = 230,
    QCA_NL80211_VENDORSUBCMD_AC_PARAMS = 231,
    QCA_NL80211_VENDORSUBCMD_RC_PARAMS_SETRTPARAMS = 232,
    QCA_NL80211_VENDORSUBCMD_SETFILTER = 233,
    QCA_NL80211_VENDORSUBCMD_MAC_COMMANDS = 234,
    QCA_NL80211_VENDORSUBCMD_WMM_PARAMS = 235,
    QCA_NL80211_VENDORSUBCMD_COUNTRY_CONFIG = 236,
    QCA_NL80211_VENDORSUBCMD_HWADDR_CONFIG = 237,
    QCA_NL80211_VENDORSUBCMD_AGGR_BURST_CONFIG = 238,
    QCA_NL80211_VENDORSUBCMD_ATF_SCHED_DURATION_CONFIG = 239,
    QCA_NL80211_VENDORSUBCMD_TXRX_PEER_STATS = 240,
    QCA_NL80211_VENDORSUBCMD_CHANNEL_CONFIG = 241,
    QCA_NL80211_VENDORSUBCMD_SSID_CONFIG = 242,
    QCA_NL80211_VENDOR_SUBCMD_PHYERR = 243,
    QCA_NL80211_VENDORSUBCMD_MGMT_RSSI_THR = 244,
    QCA_NL80211_VENDORSUBCMD_GET_SSID = 245,
    QCA_NL80211_VENDOR_SUBCMD_GET_ACLMAC = 246,
    QCA_NL80211_VENDORSUBCMD_DP_FW_PEER_STATS = 247,
    QCA_NL80211_VENDORSUBCMD_SEND_MGMT = 248,
    QCA_NL80211_VENDORSUBCMD_CHECK_11H = 249,
    QCA_NL80211_VENDOR_SUBCMD_HTTSTATS = 250,

};

/*
 * Station information block; the mac address is used
 * to retrieve other data like stats, unicast key, etc.
 */


 /**
 * enum wlan_band_id - Operational wlan band id
 * @WLAN_BAND_UNSPECIFIED: Band id not specified. Default to 2GHz/5GHz band
 * @WLAN_BAND_2GHZ: Band 2.4 GHz
 * @WLAN_BAND_5GHZ: Band 5 GHz
 * @WLAN_BAND_6GHZ: Band 6 GHz
 * @WLAN_BAND_MAX:  Max supported band
 */
enum wlan_band_id {
    WLAN_BAND_UNSPECIFIED = 0,
    WLAN_BAND_2GHZ = 1,
    WLAN_BAND_5GHZ = 2,
    WLAN_BAND_6GHZ = 3,
    /* Add new band definitions here */
    WLAN_BAND_MAX,
};
	
#define IEEE80211_ADDR_LEN      6
#define IEEE80211_RATE_MAXSIZE  44              /* max rates we'll handle */
#define MAX_NUM_OPCLASS_SUPPORTED       64
#define HEHANDLE_CAP_TXRX_MCS_NSS_SIZE  3
#define HEHANDLE_CAP_PHYINFO_SIZE       3
#define RRM_CAPS_LEN 5
#define IEEE80211_RATE_VAL              0x7f
#define IEEE80211_SEQ_SEQ_MASK              0xfff0
#define IEEE80211_SEQ_SEQ_SHIFT             4
#define MAX_CHAINS 4
#define WME_NUM_AC              4       /* 4 AC categories */
#define IEEE80211_RU_INDEX_MAX 6
#define IEEE80211_DOT11_MAX 5
#define IEEE80211_MAX_MCS (14 + 1)

struct ieee80211req_sta_info {
        u_int16_t       isi_len;                /* length (mult of 4) */
        u_int16_t       isi_freq;               /* MHz */
        int32_t         isi_nf;                 /* noise floor */
        u_int16_t       isi_ieee;               /* IEEE channel number */
        u_int32_t       awake_time;             /* time is active mode */
        u_int32_t       ps_time;                /* time in power save mode */
        u_int64_t       isi_flags;              /* channel flags */
        u_int16_t       isi_state;              /* state flags */
        u_int8_t        isi_authmode;           /* authentication algorithm */
        int8_t          isi_rssi;
    int8_t          isi_min_rssi;
    int8_t          isi_max_rssi;
        u_int16_t       isi_capinfo;            /* capabilities */
        u_int16_t       isi_pwrcapinfo;         /* power capabilities */
        u_int8_t        isi_athflags;           /* Atheros capabilities */
        u_int8_t        isi_erp;                /* ERP element */
    u_int8_t    isi_ps;         /* psmode */
        u_int8_t        isi_macaddr[IEEE80211_ADDR_LEN];
        u_int8_t        isi_nrates;
                                                /* negotiated rates */
        u_int8_t        isi_rates[IEEE80211_RATE_MAXSIZE];
        u_int8_t        isi_txrate;             /* index to isi_rates[] */
    u_int32_t   isi_txratekbps; /* tx rate in Kbps, for 11n */
        u_int16_t       isi_ie_len;             /* IE length */
        u_int16_t       isi_associd;            /* assoc response */
        u_int16_t       isi_txpower;            /* current tx power */
        u_int16_t       isi_vlan;               /* vlan tag */
        u_int16_t       isi_txseqs[17];         /* seq to be transmitted */
        u_int16_t       isi_rxseqs[17];         /* seq previous for qos frames*/
        u_int16_t       isi_inact;              /* inactivity timer */
        u_int8_t        isi_uapsd;              /* UAPSD queues */
        u_int8_t        isi_opmode;             /* sta operating mode */
        u_int8_t        isi_cipher;
        u_int32_t       isi_assoc_time;         /* sta association time */
        struct timespec isi_tr069_assoc_time;   /* sta association time in timespec format */


    u_int16_t   isi_htcap;      /* HT capabilities */
    u_int32_t   isi_rxratekbps; /* rx rate in Kbps */
                                /* We use this as a common variable for legacy rates
                                   and lln. We do not attempt to make it symmetrical
                                   to isi_txratekbps and isi_txrate, which seem to be
                                   separate due to legacy code. */
        /* XXX frag state? */
        /* variable length IE data */
    u_int32_t isi_maxrate_per_client; /* Max rate per client */
        u_int16_t   isi_stamode;        /* Wireless mode for connected sta */
    u_int32_t isi_ext_cap;              /* Extended capabilities */
    u_int32_t isi_ext_cap2;              /* Extended capabilities 2 */
    u_int32_t isi_ext_cap3;              /* Extended capabilities 3 */
    u_int32_t isi_ext_cap4;              /* Extended capabilities 4 */
    u_int8_t isi_nss;         /* number of tx and rx chains */
    u_int8_t isi_supp_nss;    /* number of tx and rx chains supported */
    u_int8_t isi_is_256qam;    /* 256 QAM support */
    u_int8_t isi_operating_bands : 2; /* Operating bands */
#if ATH_SUPPORT_EXT_STAT
    u_int8_t  isi_chwidth;            /* communication band width */
    u_int32_t isi_vhtcap;             /* VHT capabilities */
#endif
#if ATH_EXTRA_RATE_INFO_STA
    u_int8_t isi_tx_rate_mcs;
    u_int8_t isi_tx_rate_flags;
    u_int8_t isi_rx_rate_mcs;
    u_int8_t isi_rx_rate_flags;
#endif
    u_int8_t isi_rrm_caps[RRM_CAPS_LEN];    /* RRM capabilities */
    u_int8_t isi_curr_op_class;
    u_int8_t isi_num_of_supp_class;
    u_int8_t isi_supp_class[MAX_NUM_OPCLASS_SUPPORTED];
    u_int8_t isi_nr_channels;
    u_int8_t isi_first_channel;
    u_int16_t isi_curr_mode;
    u_int8_t isi_beacon_measurement_support;
    enum wlan_band_id isi_band; /* band info: 2G,5G,6G */
    u_int8_t isi_is_he;
    u_int16_t isi_hecap_rxmcsnssmap[HEHANDLE_CAP_TXRX_MCS_NSS_SIZE];
    u_int16_t isi_hecap_txmcsnssmap[HEHANDLE_CAP_TXRX_MCS_NSS_SIZE];
    u_int32_t isi_hecap_phyinfo[HEHANDLE_CAP_PHYINFO_SIZE];
};


/*
 * Per/node (station) statistics available when operating as an AP.
 */
struct ieee80211_nodestats {
    /* All below fields are moved to cp_stats component */
    int8_t       ns_rx_mgmt_rssi;         /* mgmt frame rssi */
    u_int32_t    ns_rx_mgmt;              /* rx management frames */
    u_int32_t    ns_rx_noprivacy;         /* rx w/ wep but privacy off */
    u_int32_t    ns_rx_wepfail;           /* rx wep processing failed */
    u_int32_t    ns_rx_ccmpmic;           /* rx CCMP MIC failure */
    u_int32_t    ns_rx_wpimic;            /* rx WAPI MIC failure */
    u_int32_t    ns_rx_tkipicv;           /* rx ICV check failed (TKIP) */
    u_int32_t    ns_tx_mgmt;              /* tx management frames */
    u_int32_t    ns_is_tx_not_ok;         /* tx not ok */
    u_int32_t    ns_ps_discard;           /* ps discard 'cuz of age */
    u_int32_t    ns_last_rx_mgmt_rate;         /* last received mgmt frame rate */
    u_int32_t    ns_psq_drops;           /* power save queue drops */
#ifdef ATH_SUPPORT_IQUE
    u_int32_t    ns_tx_dropblock;
#endif
    u_int32_t    ns_tx_assoc;            /* [re]associations */
    u_int32_t    ns_tx_assoc_fail;       /* [re]association failures */
    u_int32_t    ns_is_tx_nobuf;
    u_int32_t    ns_rx_decryptcrc;        /* rx decrypt failed on crc */
    /* end of cp stats fields */
    u_int32_t    ns_rx_data;             /* rx data frames */

    u_int32_t    ns_rx_ctrl;             /* rx control frames */
    u_int32_t    ns_rx_ucast;            /* rx unicast frames */
    u_int32_t    ns_rx_mcast;            /* rx multi/broadcast frames */
    u_int64_t    ns_rx_bytes;            /* rx data count (bytes) */
    u_int64_t    ns_last_per;            /* last packet error rate */
    u_int64_t    ns_rx_beacons;          /* rx beacon frames */
    u_int32_t    ns_rx_proberesp;        /* rx probe response frames */

    u_int32_t    ns_rx_dup;              /* rx discard 'cuz dup */
    u_int32_t    ns_rx_demicfail;        /* rx demic failed */

    /* We log MIC and decryption failures against Transmitter STA stats.
       Though the frames may not actually be sent by STAs corresponding
       to TA, the stats are still valuable for some customers as a sort
       of rough indication.
       Also note that the mapping from TA to STA may fail sometimes. */
    u_int32_t    ns_rx_tkipmic;           /* rx TKIP MIC failure */
    u_int32_t    ns_rx_decap;            /* rx decapsulation failed */
    u_int32_t    ns_rx_defrag;           /* rx defragmentation failed */
    u_int32_t    ns_rx_disassoc;         /* rx disassociation */
    u_int32_t    ns_rx_deauth;           /* rx deauthentication */
    u_int32_t    ns_rx_action;           /* rx action */
    u_int32_t    ns_rx_unauth;           /* rx on unauthorized port */
    u_int32_t    ns_rx_unencrypted;      /* rx unecrypted w/ privacy */

    u_int32_t    ns_tx_data;             /* tx data frames */
    u_int32_t    ns_tx_data_success;     /* tx data frames successfully
                                            transmitted (unicast only) */
    u_int64_t    ns_tx_wme[4];           /* tx data per AC */
    u_int64_t    ns_rx_wme[4];           /* rx data per AC */
    u_int32_t    ns_tx_ucast;            /* tx unicast frames */
    u_int32_t    ns_tx_mcast;            /* tx multi/broadcast frames */
    u_int32_t    ns_tx_bcast;            /* tx broadcast frames */
    u_int64_t    ns_tx_bytes;            /* tx data count (bytes) */
    u_int64_t    ns_tx_bytes_success;    /* tx success data count - unicast only
                                            (bytes) */
    u_int32_t    ns_tx_probereq;         /* tx probe request frames */
    u_int32_t    ns_tx_uapsd;            /* tx on uapsd queue */
    u_int32_t    ns_tx_discard;          /* tx dropped by NIC */
    u_int32_t    ns_tx_novlantag;        /* tx discard 'cuz no tag */
    u_int32_t    ns_tx_vlanmismatch;     /* tx discard 'cuz bad tag */

    u_int32_t    ns_tx_eosplost;         /* uapsd EOSP retried out */

    u_int32_t    ns_uapsd_triggers;      /* uapsd triggers */
    u_int32_t    ns_uapsd_duptriggers;    /* uapsd duplicate triggers */
    u_int32_t    ns_uapsd_ignoretriggers; /* uapsd duplicate triggers */
    u_int32_t    ns_uapsd_active;         /* uapsd duplicate triggers */
    u_int32_t    ns_uapsd_triggerenabled; /* uapsd duplicate triggers */
    u_int32_t    ns_last_tx_rate;         /* last tx data rate */
    u_int32_t    ns_last_rx_rate;         /* last received data frame rate */

    /* MIB-related state */
    u_int32_t    ns_dot11_tx_bytes;
    u_int32_t    ns_dot11_rx_bytes;
#if ATH_SUPPORT_EXT_STAT
    u_int32_t    ns_tx_bytes_rate;
    u_int32_t    ns_tx_data_rate;
    u_int32_t    ns_rx_bytes_rate;
    u_int32_t    ns_rx_data_rate;
    u_int32_t    ns_tx_bytes_success_last;
    u_int32_t    ns_tx_data_success_last;
    u_int32_t    ns_rx_bytes_last;
    u_int32_t    ns_rx_data_last;
#endif
    u_int32_t    ns_tx_auth;             /* [re]authentications */
    u_int32_t    ns_tx_auth_fail;        /* [re]authentication failures*/
    u_int32_t    ns_tx_deauth;           /* deauthentications */
    u_int32_t    ns_tx_deauth_code;      /* last deauth reason */
    u_int32_t    ns_tx_disassoc;         /* disassociations */
    u_int32_t    ns_tx_disassoc_code;    /* last disassociation reason */
    u_int32_t    ns_rssi_chain[MAX_CHAINS]; /* Ack RSSI per chain */
    u_int32_t   inactive_time;
    u_int32_t   ns_tx_mu_blacklisted_cnt;  /* number of time MU tx has been blacklisted */
    u_int64_t   ns_excretries[WME_NUM_AC]; /* excessive retries */
#if UMAC_SUPPORT_STA_STATS_ENHANCEMENT
    u_int64_t    ns_rx_ucast_bytes;      /* tx bytes of unicast frames */
    u_int64_t    ns_rx_mcast_bytes;      /* tx bytes of multi/broadcast frames */
    u_int64_t    ns_tx_ucast_bytes;      /* tx bytes of unicast frames */
    u_int64_t    ns_tx_mcast_bytes;      /* tx bytes of multi/broadcast frames */
    u_int64_t    ns_tx_bcast_bytes;      /* tx bytes of broadcast frames */
#endif
    u_int64_t    ns_rx_mpdus;            /* Number of rs mpdus */
    u_int64_t    ns_rx_ppdus;            /* Number of ppdus */
    u_int64_t    ns_rx_retries;          /* Number of rx retries */
    u_int32_t    ns_ppdu_tx_rate;        /* Avg per ppdu tx rate in kbps */
    u_int32_t    ns_ppdu_rx_rate;        /* Avg per ppdu rx rate in kbps */
    u_int32_t    ns_rssi;                /* most recent received packet RSSI from connected sta used for MAP*/
    u_int32_t    ns_rssi_time_delta;     /* time delta in ms between rssi measurement and IOCTL call */
    /* No of packets not trans successfully due to no of retrans attempts exceeding 802.11 retry limit */
    u_int32_t ns_failed_retry_count;
    /* No of packets that were successfully transmitted after one or more retransmissions */
    u_int32_t ns_retry_count;
    /* No of packets that were successfully transmitted after more than one retransmission */
    u_int32_t ns_multiple_retry_count;
    u_int32_t ru_tx[IEEE80211_RU_INDEX_MAX];
    u_int32_t mcs_tx[IEEE80211_DOT11_MAX][IEEE80211_MAX_MCS];
    u_int32_t ru_rx[IEEE80211_RU_INDEX_MAX];
    u_int32_t mcs_rx[IEEE80211_DOT11_MAX][IEEE80211_MAX_MCS];

#ifdef VDEV_PEER_PROTOCOL_COUNT
    u_int16_t icmp_tx_ingress;
    u_int16_t icmp_tx_egress;
    u_int16_t icmp_rx_ingress;
    u_int16_t icmp_rx_egress;

    u_int16_t arp_tx_ingress;
    u_int16_t arp_tx_egress;
    u_int16_t arp_rx_ingress;
    u_int16_t arp_rx_egress;

    u_int16_t eap_tx_ingress;
    u_int16_t eap_tx_egress;
    u_int16_t eap_rx_ingress;
    u_int16_t eap_rx_egress;
#endif
    /* TWT stats */
    u_int32_t ns_tx_data_success_twt;     /* tx data frames successfully TX */
    u_int32_t ns_rx_data_twt;             /* rx data frames */
    u_int32_t ns_twt_event_type; /* TWT session type */
    u_int32_t ns_twt_flow_id:16, /* TWT flow id */
              ns_twt_bcast:1,    /* Broadcast TWT */
              ns_twt_trig:1,     /* TWT trigger */
              ns_twt_announ:1;   /* TWT announcement */
    u_int32_t ns_twt_dialog_id;  /* TWT diag ID */
    u_int32_t ns_twt_wake_dura_us; /* Wake time duration in us */
    u_int32_t ns_twt_wake_intvl_us; /* Interval between wake perions in us */
    u_int32_t ns_twt_sp_offset_us;  /* Time until first TWT SP occurs */
};


/*
 * Retrieve per-node statistics.
 */
struct ieee80211req_sta_stats {
	union {
		/* NB: explicitly force 64-bit alignment */
		u_int8_t	macaddr[IEEE80211_ADDR_LEN];
		u_int64_t	pad;
	} is_u;
	struct ieee80211_nodestats is_stats;
};
#endif
#endif /* __LINUX_NL80211_H */
