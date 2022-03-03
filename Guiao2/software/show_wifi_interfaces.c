/*
 * This code was adapted for teaching purposes from the C code examples of the libnl Python port.
 * Origin: https://github.com/Robpol86/libnl.git
 *
 *  */
 
 /* QUESTÕES *
a. Identifique em que linha está identificado o nome da interface wireless onde abrir a socket netlink; altere para o nome da interface wireless obtido no ponto 4.
int if_index = if_nametoindex("wlp3s0"); -- linha 68

b. Identifique onde estão as funções de callback do código;
nl_socket_modify_cb(socket, NL_CB_VALID, NL_CB_CUSTOM, callback, NULL); -- linha 66

c. Identifique qual o comando Netlink enviado (i.e., estará no formato “NL80211_CMD_...”;
NL80211_CMD_GET_INTERFACE

d. Identifique qual o atributo adicionado ao comando antes dele ser enviado
NL80211_ATTR_IFINDEX

e. Na função de callback, identifique quais os atributos que são sondados
struct nl_msg *msg, void *arg

 */
 
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <linux/nl80211.h>

static int callback(struct nl_msg *msg, void *arg) {
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
    unsigned int *list = arg;

    // This parses `msg` into the `tb_msg` array with pointers.
    nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

    if (list)
        printf("List mode, no interface specified.\n");
    if (tb_msg[NL80211_ATTR_IFNAME]) 
        printf("NL80211_ATTR_IFNAME: Interface %s\n", nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));
    if (tb_msg[NL80211_ATTR_WIPHY])
        printf("NL80211_ATTR_WIPHY: wiphy %d\n", nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]));
    if (tb_msg[NL80211_ATTR_MAC]) {
        char mac_addr[20];
        mac_addr_n2a(mac_addr, nla_data(tb_msg[NL80211_ATTR_MAC]));
        printf("NL80211_ATTR_MAC: MAC address: %s\n", mac_addr);
    }
    if (tb_msg[NL80211_ATTR_IFINDEX]) 
        printf("NL80211_ATTR_IFINDEX: %d\n", nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]));
    if (tb_msg[NL80211_ATTR_IFTYPE]) 
        printf("NL80211_ATTR_IFTYPE: %d\n", nla_get_u32(tb_msg[NL80211_ATTR_IFTYPE]));
    if (tb_msg[NL80211_ATTR_WIPHY_FREQ]) 
        printf("NL80211_ATTR_WIPHY_FREQ: %d\n", nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_FREQ]));
    if (tb_msg[NL80211_ATTR_CHANNEL_WIDTH]) 
        printf("NL80211_ATTR_CHANNEL_WIDTH: %d\n", nla_get_u32(tb_msg[NL80211_ATTR_CHANNEL_WIDTH]));
    if (tb_msg[NL80211_ATTR_CENTER_FREQ1]) 
        printf("NL80211_ATTR_CENTER_FREQ1: %d\n", nla_get_u32(tb_msg[NL80211_ATTR_CENTER_FREQ1]));
    if (tb_msg[NL80211_ATTR_WIPHY_CHANNEL_TYPE]) 
        printf("NL80211_ATTR_WIPHY_CHANNEL_TYPE: %d\n", nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_CHANNEL_TYPE]));
    if (tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]) 
        printf("NL80211_ATTR_WIPHY_TX_POWER_LEVEL: %d\n", nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]));
    printf("\n");

    printf("------------------------------\n");

    return NL_SKIP;
}

int main() {
    struct nl_msg *msg;
    int ret;

    // Open socket to kernel.
    struct nl_sock *socket = nl_socket_alloc();  // Allocate new netlink socket in memory.
    genl_connect(socket);  // Create file descriptor and bind socket.
    int driver_id = genl_ctrl_resolve(socket, "nl80211");  // Find the nl80211 driver ID.

    // First we'll get info for wlan0. -- trocar pelo nome obtido da interface wlp3s0
    printf(">>> Getting info for wlp3s0:\n");
    nl_socket_modify_cb(socket, NL_CB_VALID, NL_CB_CUSTOM, callback, NULL);
    msg = nlmsg_alloc();  // Allocate a message.
    int if_index = if_nametoindex("wlp3s0");
    genlmsg_put(msg, 0, 0, driver_id, 0, 0, NL80211_CMD_GET_INTERFACE, 0);  // Setup the message.
    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, if_index);  // Add message attributes.
    ret = nl_send_auto_complete(socket, msg);  // Send the message.
    nl_recvmsgs_default(socket);  // Retrieve the kernel's answer.
    nl_wait_for_ack(socket);

    printf(">>> Program exit.\n");
    return 0;

    // Goto statement required by NLA_PUT_U32().
    nla_put_failure:
        nlmsg_free(msg);
        return 1;
}

void mac_addr_n2a(char *mac_addr, unsigned char *arg) {
    // From http://git.kernel.org/cgit/linux/kernel/git/jberg/iw.git/tree/util.c.
    int i, l;

    l = 0;
    for (i = 0; i < 6; i++) {
        if (i == 0) {
            sprintf(mac_addr+l, "%02x", arg[i]);
            l += 2;
        } else {
            sprintf(mac_addr+l, ":%02x", arg[i]);
            l += 3;
        }
    }
}

