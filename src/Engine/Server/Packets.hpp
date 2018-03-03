#ifndef GAME_PACKETS
  #define GAME_PACKETS

  const UInt16 PACKET_EMPTY               = 0;
  /*
    empty
  */
  const UInt16 PACKET_CONNECT_CONFIRM     = 1;
  /*
    UINT16       -       Version
  */
  const UInt16 PACKET_CLIENT_DROPPED      = 2;
  /*
    STRING       -       REASON
  */
  const UInt16 PACKET_PING                = 3;
  /*
    EMPTY
  */
  const UInt16 PACKET_EXIT                = 4;
  /*
    EMPTY
  */

#endif
