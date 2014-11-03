
        printk("Configuring static IP... ");
        net_ip_t ip, mask, gateway;
        net_set_ip(&ip, __NETWORK_IP_ADDR1__,__NETWORK_IP_ADDR2__,__NETWORK_IP_ADDR3__,__NETWORK_IP_ADDR4__);
        net_set_ip(&mask, __NETWORK_IP_MASK1__,__NETWORK_IP_MASK2__,__NETWORK_IP_MASK3__,__NETWORK_IP_MASK4__);
        net_set_ip(&gateway, __NETWORK_IP_GW1__,__NETWORK_IP_GW2__,__NETWORK_IP_GW3__,__NETWORK_IP_GW4__);
        if (net_ifup(&ip, &mask, &gateway) == 0) {
                printk("OK\n");
        } else {
                printk(FONT_COLOR_RED"fail"RESET_ATTRIBUTES"\n");
        }
