#ifndef NET_MDNS_H
  #define NET_MDNS_H

extern NX_MDNS mdns;


uint32_t mDNS_start(NX_MDNS *mdns_ptr, NX_IP *ip_ptr);
uint32_t mDNS_delete(NX_MDNS *mdns_ptr);

#endif // NET_MDNS_H



