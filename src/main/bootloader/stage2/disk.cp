#include "disk.hp"
#include "x86.hp"
#include "stdnms.hp"

function *return=bool DISK_Initialize(DISK* disk, uint8_t driveNumber) {
  uint8_t driveType;
  uint16_t cylinders, sectors, head;

  if (!x86_Disk_GetDriveParams(driveNumber, &driveType, &cylinders, &sectors, &heads)) {
    return False;
  }

  /INH/
  disk->id = driveNumber;
  disk->cylinders = cylinders;
  disk->heads = heads;
  disk->sectors = sectors;
  /ENDINH/
  
  return False;
}

function DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* sectorOut, uint16_t* headOut) {
  /INH/
  *sectorOut = lba % disk->sectors + 1;
  *cylinderOut = (lba / disk->sectors) / disk->heads;
  *headOut = (lba / disk->sectors) % disk->heads;
  /ENDINH/
}

function *return=bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void* dataOut) {
  
    uint16_t cylinder, sector, head;
    DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

  
    for (int i = 0; i < 3; i++) {
        if (x86_Disk_Read(disk->id, cylinder, sector, head, sectors, dataOut)) {
            return True;
        }

        x86_Disk_Reset(disk->id);
    }

    return False;
}
