#include "netlink/netlink.h"
#include "netlink/genl/genl.h"
#include "netlink/genl/ctrl.h"
#include <net/if.h>

//copy this from iw
#include <linux/nl80211.h>

static int expectedId;

static int nlCallback(struct nl_msg* msg, void* arg)
{
    struct nlmsghdr* ret_hdr = nlmsg_hdr(msg); //returns pointer to received message
    struct nlattr *tb_msg[NL80211_ATTR_MAX + 1]; //attributes array

    if (ret_hdr->nlmsg_type != expectedId)
    {
        return NL_STOP;
    }

    //D: Interprets the netlink protocol payload as a generic link header
    struct genlmsghdr *gnlh = (struct genlmsghdr*) nlmsg_data(ret_hdr);

    /*D: Parses the message's attributes,
        Parameters
            tb      Index array to be filled (maxtype+1 elements).
            maxtype	Maximum attribute type expected and accepted.
            head	Head of attribute stream.
            len     Length of attribute stream.
            policy	Attribute validation policy.
    */
    nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
              genlmsg_attrlen(gnlh, 0), NULL);

    printf("----------------------------------------------------------\n");
    printf("%d\n",sizeof(tb_msg));

    

    printf("----------------------------------------------------------\n");
    
    if (tb_msg[NL80211_ATTR_REG_ALPHA2]) 
        printf("NL80211_ATTR_REG_ALPHA2: %s\n", nla_get_string(tb_msg[NL80211_ATTR_REG_ALPHA2]));
        
    if(tb_msg[NL80211_ATTR_REG_RULES]){
        struct nlattr * nested[NL80211_MAX_SUPP_REG_RULES+1];
        int err = nla_parse_nested(nested,NL80211_MAX_SUPP_REG_RULES, tb_msg[NL80211_ATTR_REG_RULES],NULL);
        
/*        struct nlattr * inner[NL80211_MAX_SUPP_REG_RULES+1];
        int err2 = nla_parse_nested(inner,NL80211_MAX_SUPP_REG_RULES,nested[0],NULL);
        
        if(inner[NL80211_ATTR_REG_RULE_FLAGS]){
            printf("Reg Rule Flags: ");
            int type = nla_get_u32(inner[NL80211_ATTR_REG_RULE_FLAGS]);
            printf("%d\n",type);
        }
        if(inner[NL80211_ATTR_FREQ_RANGE_START]){
            printf("Range Start: ");
            int type = nla_get_u32(inner[NL80211_ATTR_FREQ_RANGE_START]);
            printf("%d\n",type);
        }
        if(inner[NL80211_ATTR_FREQ_RANGE_END]){
            printf("Range End: ");
            int type = nla_get_u32(inner[NL80211_ATTR_FREQ_RANGE_END]);
            printf("%d\n",type);
        }
        if(inner[NL80211_ATTR_FREQ_RANGE_MAX_BW]){
            printf("Range Max BW: ");
            int type = nla_get_u32(inner[NL80211_ATTR_FREQ_RANGE_MAX_BW]);
            printf("%d\n",type);
        }
        if(inner[NL80211_ATTR_DFS_CAC_TIME]){
            printf("DFS CAC time: ");
            int type = nla_get_u32(inner[NL80211_ATTR_DFS_CAC_TIME]);
            printf("%d\n",type);
        }
*/

        struct nlattr * nl_rule;
        int rem_rule;
        nla_for_each_nested(nl_rule, tb_msg[NL80211_ATTR_REG_RULES],rem_rule){
            struct nlattr * tb_rule[NL80211_REG_RULE_ATTR_MAX + 1];
            int flags, start_freq_khz, end_freq_khz, max_bw_khz;
            nla_parse(tb_rule, NL80211_REG_RULE_ATTR_MAX, nla_data(nl_rule),nla_len(nl_rule), NULL);
            start_freq_khz = nla_get_u32(tb_rule[NL80211_ATTR_FREQ_RANGE_START]);
            printf("freq: %d\n",start_freq_khz);
        }

    }
    
}

int main(int argc, char** argv)
{
    int ret;
    //allocate socket
    struct nl_sock* sk = nl_socket_alloc();

    //connect to generic netlink socket
    genl_connect(sk);

    //find the nl80211 driver ID
    expectedId = genl_ctrl_resolve(sk, "nl80211");

    //attach a callback
    nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM,
            nlCallback, NULL);

    //allocate a message
    struct nl_msg* msg = nlmsg_alloc();

    enum nl80211_commands* cmd = NL80211_CMD_GET_REG;
    int ifIndex = if_nametoindex("wlp3s0");
    int flags = 0;

    // setup the message (adds generic netlink headers)
    genlmsg_put(msg, 0, 0, expectedId, 0, flags, cmd, 0);

    //add message attributes (indicate the operating interface of the device)
    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifIndex);

    //send the messge (this frees it)
    ret = nl_send_auto_complete(sk, msg);

    //block for message to return
    nl_recvmsgs_default(sk);

    return 0;

nla_put_failure:
    nlmsg_free(msg);
    return 1;
}
