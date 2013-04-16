/*================================================================================*/
/*                              ictOS ATA I/O Ports                               */
/*                                                                        by: ict */
/*================================================================================*/

/* Basic Information */
#define ATA_SECTORSIZE          0x200

/* Device Number */
#define ATA_PRIMARY             0x0
#define ATA_SLAVE               0x1

/* Primary */
#define ATA_P_DATA              0x1f0
#define ATA_P_ERROR             0x1f1
#define ATA_P_FEATURES          0x1f1
#define ATA_P_SECTORCOUNT       0x1f2
#define ATA_P_LBALOW            0x1f3
#define ATA_P_SECTORNUMBER      0x1f3
#define ATA_P_LBAMID            0x1f4
#define ATA_P_CYLINDERLOW       0x1f4
#define ATA_P_LBAHIGH           0x1f5
#define ATA_P_CYLINDERHIGH      0x1f5
#define ATA_P_DEVICE            0x1f6
#define ATA_P_STATUS            0x1f7
#define ATA_P_COMMAND           0x1f7
#define ATA_P_ALTERNATESTATUS   0x3f6
#define ATA_P_DEVICECONTROL     0x3f6

/* Secondary */
#define ATA_S_DATA              0x170
#define ATA_S_ERROR             0x171
#define ATA_S_FEATURES          0x171
#define ATA_S_SECTORCOUNT       0x172
#define ATA_S_LBALOW            0x173
#define ATA_S_LBAMID            0x174
#define ATA_S_LBAHIGH           0x175
#define ATA_S_DEVICE            0x176
#define ATA_S_STATUS            0x177
#define ATA_S_COMMAND           0x177
#define ATA_S_ALTERNATESTATUS   0x376
#define ATA_S_DEVICECONTROL     0x376

/* Device Register */
#define ATA_DEVREG              0xa0
#define ATA_DEVREG_LBA          0x40
#define ATA_DEVREG_MASTER       0x00
#define ATA_DEVREG_SLAVE        0x10

/* Status Register */
#define ATA_STATUS_BSY          0x80
#define ATA_STATUS_DRDY         0x40
#define ATA_STATUS_DFSE         0x20
#define ATA_STATUS_DRQ          0x08
#define ATA_STATUS_ERR          0x01

/* Device Control Register */
#define ATA_DEVCTRLREG_HOB      0x80
#define ATA_DEVCTRLREG_SRST     0x04
#define ATA_DEVCTRLREG_IEN      0x02

/* Command */
#define ATA_CMD_IDENTIFY        0xec
#define ATA_CMD_READSECTORS     0x20
#define ATA_CMD_WRITESECTORS    0x30
