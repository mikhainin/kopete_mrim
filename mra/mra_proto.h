//***************************************************************************
// $Id: proto.h,v 1.1 2008/08/14 15:39:44 builov Exp $
//***************************************************************************

#ifndef MRIM_PROTO_H
#define MRIM_PROTO_H

#include <sys/types.h>
#include <stdint.h>
#define PROTO_VERSION_MAJOR     1
// and then protocol has been changed. Even login is impossible
#define PROTO_VERSION_MINOR     23
#define MAKE_VERSION(major, minor) ((((uint32_t)(major))<<16)|(uint32_t)(minor))
#define PROTO_VERSION MAKE_VERSION(PROTO_VERSION_MAJOR,PROTO_VERSION_MINOR)


#define PROTO_MAJOR(p) (((p)&0xFFFF0000)>>16)
#define PROTO_MINOR(p) ((p)&0x0000FFFF)

#pragma pack(push,1)
typedef struct mrim_packet_header_t
{
    uint32_t      magic;		// Magic
    uint32_t      proto;		// Protocol's version
    uint32_t      seq;		// Sequence
    uint32_t      msg;		// packet type
    uint32_t      dlen; 		// Data length
    uint32_t      from;		// Sender's address
    uint32_t      fromport;	// Sender's port
    u_char	reserved[16];	// Reserved (not used yet)
}
mrim_packet_header_t;
#pragma pack(pop)

typedef uint32_t mrim_msg_t;

#define CS_MAGIC    0xDEADBEEF		// Client's magick ( C <-> S )


/***************************************************************************

        The client-server protocol

 ***************************************************************************/

#define MRIM_CS_HELLO       	0x1001  // C -> S
    // empty

#define MRIM_CS_HELLO_ACK   	0x1002  // S -> C
    // // mrim_connection_params_t

    // added by negram:
    // UL ping seconts
    // UIDL ??? (example: d1 9e 80 4f 38 17 5e)

#define MRIM_CS_LOGIN_ACK   	0x1004  // S -> C
    // empty

#define MRIM_CS_LOGIN_REJ   	0x1005  // S -> C
    // LPS reason

#define MRIM_CS_PING        	0x1006  // C -> S
    // empty

#define MRIM_CS_MESSAGE			0x1008  // C -> S
    // UL flags
    // LPS to
    // LPS message
    // LPS rtf-formatted message (>=1.1)
    #define MESSAGE_FLAG_OFFLINE		0x00000001
    #define MESSAGE_FLAG_NORECV                 0x00000004
    #define MESSAGE_FLAG_AUTHORIZE		0x00000008 	// X-MRIM-Flags: 00000008
    #define MESSAGE_FLAG_SYSTEM                 0x00000040
    #define MESSAGE_FLAG_RTF                    0x00000080
    #define MESSAGE_FLAG_CONTACT		0x00000200
    #define MESSAGE_FLAG_NOTIFY                 0x00000400
    #define MESSAGE_FLAG_MULTICAST		0x00001000

    #define MESSAGE_FLAG_UNICODE                0x00100000
    #define MESSAGE_FLAG_CHAT                   0x00400000

#define MAX_MULTICAST_RECIPIENTS 50
    #define MESSAGE_USERFLAGS_MASK	0x000036A8	// Flags that user is allowed to set himself


#define MRIM_CS_MESSAGE_ACK		0x1009  // S -> C
    // UL msg_id
    // UL flags
    // LPS from
    // LPS message
    // LPS rtf-formatted message (>=1.1)




#define MRIM_CS_MESSAGE_RECV	0x1011	// C -> S
    // LPS from
    // UL msg_id

#define MRIM_CS_MESSAGE_STATUS	0x1012	// S -> C
    // UL status
    #define MESSAGE_DELIVERED		0x0000	// Message delivered directly to user
    #define MESSAGE_REJECTED_NOUSER		0x8001  // Message rejected - no such user
    #define MESSAGE_REJECTED_INTERR		0x8003	// Internal server error
    #define MESSAGE_REJECTED_LIMIT_EXCEEDED	0x8004	// Offline messages limit exceeded
    #define MESSAGE_REJECTED_TOO_LARGE	0x8005	// Message is too large
    #define	MESSAGE_REJECTED_DENY_OFFMSG	0x8006	// User does not accept offline messages

#define MRIM_CS_USER_STATUS	0x100F	// S -> C
    // UL status
    #define STATUS_OFFLINE		0x00000000
    #define STATUS_ONLINE		0x00000001
    #define STATUS_AWAY		0x00000002
    #define STATUS_UNDETERMINATED	0x00000003
    #define STATUS_FLAG_INVISIBLE	0x80000000
    // LPS user


#define MRIM_CS_LOGOUT			0x1013	// S -> C
    // UL reason
    #define LOGOUT_NO_RELOGIN_FLAG	0x0010		// Logout due to double login

#define MRIM_CS_CONNECTION_PARAMS	0x1014	// S -> C
    // mrim_connection_params_t

#define MRIM_CS_USER_INFO			0x1015	// S -> C
    // (LPS key, LPS value)* X


#define MRIM_CS_ADD_CONTACT			0x1019	// C -> S
    // UL flags (group(2) or usual(0)
    // UL group id (unused if contact is group)
    // LPS contact
    // LPS name
    // LPS unused
    #define CONTACT_FLAG_REMOVED	0x00000001
    #define CONTACT_FLAG_GROUP	0x00000002
    #define CONTACT_FLAG_INVISIBLE	0x00000004
    #define CONTACT_FLAG_VISIBLE	0x00000008
    #define CONTACT_FLAG_IGNORE	0x00000010
    #define CONTACT_FLAG_SHADOW	0x00000020

#define MRIM_CS_ADD_CONTACT_ACK			0x101A	// S -> C
    // UL status
    // UL contact_id or (u_long)-1 if status is not OK

    #define CONTACT_OPER_SUCCESS		0x0000
    #define CONTACT_OPER_ERROR		0x0001
    #define CONTACT_OPER_INTERR		0x0002
    #define CONTACT_OPER_NO_SUCH_USER	0x0003
    #define CONTACT_OPER_INVALID_INFO	0x0004
    #define CONTACT_OPER_USER_EXISTS	0x0005
    #define CONTACT_OPER_GROUP_LIMIT	0x6

#define MRIM_CS_MODIFY_CONTACT			0x101B	// C -> S
    // UL id
    // UL flags - same as for MRIM_CS_ADD_CONTACT
    // UL group id (unused if contact is group)
    // LPS contact
    // LPS name
    // LPS unused

#define MRIM_CS_MODIFY_CONTACT_ACK		0x101C	// S -> C
    // UL status, same as for MRIM_CS_ADD_CONTACT_ACK

#define MRIM_CS_OFFLINE_MESSAGE_ACK		0x101D	// S -> C
    // UIDL
    // LPS offline message

#define MRIM_CS_DELETE_OFFLINE_MESSAGE		0x101E	// C -> S
    // UIDL


#define MRIM_CS_AUTHORIZE			0x1020	// C -> S
    // LPS user

#define MRIM_CS_AUTHORIZE_ACK			0x1021	// S -> C
    // LPS user

#define MRIM_CS_CHANGE_STATUS			0x1022	// C -> S
    // UL new status

    // v1.23 (added by negram)
    // LPS status text, e.g. status_dnd
    // LPUS status title, e.g. "Don't dustrub" in russian (or contact's language)
    // DATA[8] ???  00 00 00 00 ff 0b 00 00


#define MRIM_CS_GET_MPOP_SESSION		0x1024	// C -> S


#define MRIM_CS_MPOP_SESSION			0x1025	// S -> C
    #define MRIM_GET_SESSION_FAIL		0
    #define MRIM_GET_SESSION_SUCCESS	1
    //UL status
    // LPS mpop session


//white pages!
#define MRIM_CS_WP_REQUEST			0x1029 //C->S
//DWORD field, LPS value
#define PARAMS_NUMBER_LIMIT			50
#define PARAM_VALUE_LENGTH_LIMIT		64

//if last symbol in value eq '*' it will be replaced by LIKE '%'
// params define
// must be  in consecutive order (0..N) to quick check in check_anketa_info_request
enum {
  MRIM_CS_WP_REQUEST_PARAM_USER		= 0,
  MRIM_CS_WP_REQUEST_PARAM_DOMAIN,
  MRIM_CS_WP_REQUEST_PARAM_NICKNAME,
  MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME,
  MRIM_CS_WP_REQUEST_PARAM_LASTNAME,
  MRIM_CS_WP_REQUEST_PARAM_SEX	,
  MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY,
  MRIM_CS_WP_REQUEST_PARAM_DATE1	,
  MRIM_CS_WP_REQUEST_PARAM_DATE2	,
  //!!!!!!!!!!!!!!!!!!!online request param must be at end of request!!!!!!!!!!!!!!!
  MRIM_CS_WP_REQUEST_PARAM_ONLINE	,
  MRIM_CS_WP_REQUEST_PARAM_STATUS	,	 // we do not used it, yet
  MRIM_CS_WP_REQUEST_PARAM_CITY_ID,
  MRIM_CS_WP_REQUEST_PARAM_ZODIAC,
  MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_MONTH,
  MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_DAY,
  MRIM_CS_WP_REQUEST_PARAM_COUNTRY_ID,
  MRIM_CS_WP_REQUEST_PARAM_MAX
};

#define MRIM_CS_ANKETA_INFO			0x1028 //S->C
//DWORD status
    #define MRIM_ANKETA_INFO_STATUS_OK		1
    #define MRIM_ANKETA_INFO_STATUS_NOUSER		0
    #define MRIM_ANKETA_INFO_STATUS_DBERR		2
    #define MRIM_ANKETA_INFO_STATUS_RATELIMERR	3
//DWORD fields_num
//DWORD max_rows
//DWORD server_time sec since 1970 (unixtime)
// fields set 				//%fields_num == 0
//values set 				//%fields_num == 0
//LPS value (numbers too)


#define MRIM_CS_MAILBOX_STATUS			0x1033
//DWORD new messages in mailbox


#define MRIM_CS_CONTACT_LIST2		0x1037 //S->C
// UL status
#define GET_CONTACTS_OK			0x0000
#define GET_CONTACTS_ERROR		0x0001
#define GET_CONTACTS_INTERR		0x0002
//DWORD status  - if ...OK than this staff:
//DWORD groups number
//mask symbols table:
//'s' - lps
//'u' - unsigned long
//'z' - zero terminated string
//LPS groups fields mask
//LPS contacts fields mask
//group fields
//contacts fields
//groups mask 'us' == flags, name
//contact mask 'uussuu' flags, flags, internal flags, status
    #define CONTACT_INTFLAG_NOT_AUTHORIZED	0x0001


//old packet cs_login with cs_statistic
#define MRIM_CS_LOGIN2       	0x1038  // C -> S
#define MAX_CLIENT_DESCRIPTION 256
// LPS login
// LPS password
// DWORD status
//+ statistic packet data:
// LPS client description //max 256


#define MRIM_CS_UNKNOWN1       	0x1086  // C -> S
        // no data. Client sends to the server immediately after MRIM_CS_HELLO_ACK
        // suggestion: "start SSL session"

#define MRIM_CS_UNKNOWN1_ACK       	0x1087  // S -> C
        // no data. Server sends to the client immediately after MRIM_CS_UNKNOWN1
        // sequence number is the same as in MRIM_CS_UNKNOWN1

#define MRIM_CS_UNKNOWN2       	0x1090  // C -> S
        // after MRIM_CS_HELLO_ACK
        // UL ??? ( example:     03 00 00 00 )
        // UL ??? (perhaps, LPS, 00 00 00 00 )
        // UL ??? ( example:     00 00 00 00 )
        // UL ??? ( example:     01 00 00 00 )
        // UL ??? ( example:     00 00 00 00 )
        // UL ??? ( example:     02 00 00 00 )
        // UL ??? ( example:     00 00 00 00 )


#define MRIM_CS_LOGIN3       	0x1078  // C -> S
        // LPS login
        // LPS md5(password) (?)
        // UL status
        // LPS client description
        // LPS country code  (e.g. "ru")
        // UL ??? (e.g. 00000010)
        // UL ??? (e.g. 00000001)
        // LPS ??? (e.g. geo-list)
        // LPS client description (again ???)

        /*    many times:
              UL = x
              data:
                if x == 00 00 00 00 02:
                    data = data[5] ???
                else if x == 00 00 00 01:
                    data = LPS
        */
/*

login
0000   15 00 00 00 6d 69 6b 68 61 69 6c 2e 67 61 6c 61  ....mikhail.gala
0010   6e 69 6e 40 62 6b 2e 72 75                       nin@bk.ru

password (?)
0010                              10 00 00 00 xx xx xx           ....xxx
0020   xx xx xx xx xx xx xx xx xx xx xx xx xx           xxxxxxxxxxxxx

status (?)
0030                                          ff 0b 00               ...
0030   00                                               .

client description
          2b 00 00 00 63 6c 69 65 6e 74 3d 22 6d 61 67   +...client="mag
0040   65 6e 74 22 20 76 65 72 73 69 6f 6e 3d 22 35 2e  ent" version="5.
0050   31 30 22 20 62 75 69 6c 64 3d 22 35 32 38 32 22  10" build="5282"

Geo code
0060   02 00 00 00 72 75                                ....ru

UL ??? UL ???
0060                     10 00 00 00 01 00 00 00

LPS ???
0060                                             08 00        ..........
0070   00 00 67 65 6f 2d 6c 69 73 74                    ..geo-list

LPS client description (again ???)
0070                                 16 00 00 00 4d 52            ....MR
0080   41 20 35 2e 31 30 20 28 62 75 69 6c 64 20 35 32  A 5.10 (build 52
0090   38 32 29 3b                                      82);

many times:
  UL = x
  data:
    if x == 00 00 00 00 02:
        data = data[5] ???
    else if x == 00 00 00 01:
        data = LPS

0090               00 00 00 00 02 a2 14 00 00 01 00 00      ............

00a0   00 02 1e 00 00 00 02 00 00 00 02 12 00 00 00 03  ................

00b0   00 00 00 02 58 ca fd 2e 05 00 00 00 02 00 00 00  ....X...........

00c0   00 04 00 00 00 02 ff ff ff ff 06 00 00 00 02 ff  ................

00d0   ff ff ff 07 00 00 00 02 ff ff ff ff 08 00 00 00  ................

00e0   02 00 00 00 00 09                                ......


00e0                     00 00 00 01                          ....

00e0                                 22 00 00 00 5c 5f            "...\_
00f0   59 51 57 5b 5f 1f 53 53 5a 54 5d 59 59 73 5a 59  YQW[_.SSZT]YYsZY
0100   1d 47 42 5f 5a 5c 5a 59 51 5f 1b 57 59 5d 55 5e  .GB_Z\ZYQ_.WY]U^
0110   0a 00 00 00 02 00 00 00 00 0b 00 00 00 02 00 00  ................
0120   00 00 0c 00 00 00 02 00 00 00 00 0d 00 00 00 02  ................
0130   00 00 00 00 0e 00 00 00 02 00 00 00 00 0f 00 00  ................
0140   00 02 00 00 00 00 10 00 00 00 02 00 00 00 00 11  ................
0150   00 00 00 02 00 00 00 00 12 00 00 00 02 00 00 00  ................
0160   00 13 00 00 00 02 00 00 00 00 93 00 00 00 02 00  ................
0170   00 00 00 94 00 00 00 02 00 00 00 00 15 00 00 00  ................
0180   02 00 00 00 00 67 00 00 00 02 00 00 00 00 1e 00  .....g..........
0190   00 00 01 00 00 00 00 1f 00 00 00 02 00 00 00 00  ................
01a0   20 00 00 00 02 00 00 00 00 21 00 00 00 02 00 00   ........!......
01b0   00 00 22 00 00 00 02 00 00 00 00 2d 00 00 00 02  .."........-....
01c0   01 00 00 00 2e 00 00 00 02 00 00 00 00 81 00 00  ................
01d0   00 02 01 00 00 00 14 00 00 00 02 01 05 00 00 16  ................
01e0   00 00 00 02 00 00 00 00 17 00 00 00 02 00 00 00  ................
01f0   00 18 00 00 00 02 00 00 00 00 19 00 00 00 02 00  ................
0200   00 00 00 1a 00 00 00 02 00 00 00 00 1c 00 00 00  ................
0210   02 00 00 00 00 1d 00 00 00 02 00 00 00 00 23 00  ..............#.
0220   00 00 02 00 00 00 00 24 00 00 00 02 00 00 00 00  .......$........
0230   26 00 00 00 02 00 00 00 00 27 00 00 00 02 00 00  &........'......
0240   00 00 28 00 00 00 02 00 00 00 00 29 00 00 00 02  ..(........)....
0250   00 00 00 00 2a 00 00 00 02 00 00 00 00 2b 00 00  ....*........+..
0260   00 02 00 00 00 00 2c 00 00 00 01 20 00 00 00 65  ......,.... ...e
0270   39 35 37 36 39 30 30 63 62 31 64 33 37 38 30 34  9576900cb1d37804
0280   63 65 32 64 64 33 36 64 36 66 62 36 38 63 39 2f  ce2dd36d6fb68c9/
0290   00 00 00 02 00 00 00 00 3f 00 00 00 02 01 00 00  ........?.......
02a0   00 40 00 00 00 02 01 00 00 00 41 00 00 00 02 08  .@........A.....
02b0   07 00 00 42 00 00 00 01 4c 00 00 00 4d 00 6f 00  ...B....L...M.o.
02c0   62 00 69 00 6c 00 65 00 20 00 41 00 4d 00 44 00  b.i.l.e. .A.M.D.
02d0   20 00 53 00 65 00 6d 00 70 00 72 00 6f 00 6e 00   .S.e.m.p.r.o.n.
02e0   28 00 74 00 6d 00 29 00 20 00 50 00 72 00 6f 00  (.t.m.). .P.r.o.
02f0   63 00 65 00 73 00 73 00 6f 00 72 00 20 00 33 00  c.e.s.s.o.r. .3.
0300   30 00 30 00 30 00 2b 00 43 00 00 00 01 42 00 00  0.0.0.+.C....B..
0310   00 4d 00 69 00 63 00 72 00 6f 00 73 00 6f 00 66  .M.i.c.r.o.s.o.f
0320   00 74 00 20 00 57 00 69 00 6e 00 64 00 6f 00 77  .t. .W.i.n.d.o.w
0330   00 73 00 20 00 58 00 50 00 20 00 48 00 6f 00 6d  .s. .X.P. .H.o.m
0340   00 65 00 20 00 45 00 64 00 69 00 74 00 69 00 6f  .e. .E.d.i.t.i.o
0350   00 6e 00 44 00 00 00 01 00 00 00 00 45 00 00 00  .n.D........E...
0360   01 08 00 00 00 30 00 34 00 31 00 39 00 46 00 00  .....0.4.1.9.F..
0370   00 02 25 00 00 00 47 00 00 00 02 01 00 00 00 48  ..%...G........H
0380   00 00 00 02 00 05 00 00 49 00 00 00 02 20 03 00  ........I.... ..
0390   00 4a 00 00 00 02 20 00 00 00 4b 00 00 00 01 14  .J.... ...K.....
03a0   00 00 00 53 00 69 00 53 00 20 00 4d 00 37 00 36  ...S.i.S. .M.7.6
03b0   00 30 00 47 00 58 00 4c 00 00 00 01 14 00 00 00  .0.G.X.L........
03c0   41 00 63 00 65 00 72 00 2c 00 20 00 69 00 6e 00  A.c.e.r.,. .i.n.
03d0   63 00 2e 00 4d 00 00 00 02 de 07 00 00 4e 00 00  c...M........N..
03e0   00 02 02 00 00 00 51 00 00 00 02 00 00 00 00 52  ......Q........R
03f0   00 00 00 02 00 00 00 00 53 00 00 00 02 00 00 00  ........S.......
0400   00 54 00 00 00 02 00 00 00 00 56 00 00 00 02 00  .T........V.....
0410   00 00 00 57 00 00 00 02 00 00 00 00 58 00 00 00  ...W........X...
0420   02 00 00 00 00 59 00 00 00 02 00 00 00 00 5a 00  .....Y........Z.
0430   00 00 02 00 00 00 00 5b 00 00 00 02 00 00 00 00  .......[........
0440   5c 00 00 00 02 00 00 00 00 5d 00 00 00 02 00 00  \........]......
0450   00 00 5e 00 00 00 02 00 00 00 00 5f 00 00 00 02  ..^........_....
0460   00 00 00 00 60 00 00 00 02 00 00 00 00 61 00 00  ....`........a..
0470   00 02 00 00 00 00 62 00 00 00 02 00 00 00 00 63  ......b........c
0480   00 00 00 02 00 00 00 00 64 00 00 00 02 00 00 00  ........d.......
0490   00 65 00 00 00 02 00 00 00 00 66 00 00 00 02 00  .e........f.....
04a0   00 00 00 68 00 00 00 01 00 00 00 00 69 00 00 00  ...h........i...
04b0   02 00 00 00 00 6a 00 00 00 02 00 00 00 00 6b 00  .....j........k.
04c0   00 00 02 00 00 00 00 6c 00 00 00 02 00 00 00 00  .......l........
04d0   6d 00 00 00 02 00 00 00 00 6e 00 00 00 02 00 00  m........n......
04e0   00 00 6f 00 00 00 02 00 00 00 00 70 00 00 00 02  ..o........p....
04f0   00 00 00 00 71 00 00 00 02 00 00 00 00 72 00 00  ....q........r..
0500   00 02 00 00 00 00 73 00 00 00 02 00 00 00 00 74  ......s........t
0510   00 00 00 02 00 00 00 00 75 00 00 00 02 00 00 00  ........u.......
0520   00 76 00 00 00 02 00 00 00 00 77 00 00 00 02 00  .v........w.....
0530   00 00 00 78 00 00 00 02 00 00 00 00 79 00 00 00  ...x........y...
0540   02 00 00 00 00 7a 00 00 00 02 00 00 00 00 7c 00  .....z........|.
0550   00 00 02 00 00 00 00 7b 00 00 00 02 00 00 00 00  .......{........
0560   7d 00 00 00 02 00 00 00 00 82 00 00 00 02 00 00  }...............
0570   00 00 84 00 00 00 02 00 00 00 00 86 00 00 00 02  ................
0580   00 00 00 00 87 00 00 00 02 00 00 00 00 89 00 00  ................
0590   00 02 01 00 00 00 8a 00 00 00 02 00 00 00 00 8b  ................
05a0   00 00 00 02 00 00 00 00 8d 00 00 00 02 00 00 00  ................
05b0   00 8e 00 00 00 02 00 00 00 00 83 00 00 00 01 06  ................
05c0   00 00 00 73 70 6c 61 73 68 85 00 00 00 01 06 00  ...splash.......
05d0   00 00 73 70 6c 61 73 68                          ..splash
*/


typedef struct mrim_connection_params_t
{
        uint32_t	ping_period;
}
mrim_connection_params_t;



#endif // MRIM_PROTO_H
