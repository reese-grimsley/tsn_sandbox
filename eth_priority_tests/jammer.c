/**
 * 
 * The Jammer will inject traffic into a raw socket as quickly as it can.
 *      However, we're using a 2.5Gbps capable NIC, so it's unlikely it is anywhere near saturation..
 * 
 * Assumed platform: Ubuntu 20.04 LTS, Intel Nuc (series 11), NIC i225
 * 
 * Author: Reese Grimsley
 * Created: 10/29/21
 * 
 * 
 * Raw sockets references:
 *  https://www.binarytides.com/raw-sockets-c-code-linux/
 */
#include <stdio.h>
#include <sys/socket.h>

#include "constants.h"


int main(int argc, char* argv[])
{

}