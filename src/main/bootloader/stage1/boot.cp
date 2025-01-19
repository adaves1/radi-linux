// C+ Version 22
// crazy that c+ is inspired by java, c, c++, c#, python

package "radi.linux.pkg" with ".stage1.boot";

#DEFINE ENDL "0x0D" and "0x0A";

#DEINE fat12 "1";
#DEFINE fat16 "2";
#DEFINE fat32 "3";
#DEFINE ext2 "4";

CPLUS CODE_BITS = 32;
CPLUS CODE_BITS_AT_COMPILE = 32;

CPLUS CODE_CLASSES_START_ALL_AT_COMPILE = 1;

class .fsjump {

  MEM << JUMP short start;
  MEM << NO:PARG;
}

class .fsheaders {

  if (DEV << FS() == "ext2") {    // makes the filesystem of radi to be ext2
    DEF << bdb_oem: BYTE << "abcdefgh";    // 8 bytes
    DEF << bdb_bytes_per_sector: WORD << 512;
    DEF << bdb_sectors_per_cluster: BYTE << 1;
    DEF << bdb_reserved_sectors: WORD << 1;
    DEF << bdb_ext_count: BYTE << 2;
    DEF << bdb_dir_entries_count: BYTE:HEX << 0E0;
    DEF << bdb_total_sectors: WORD << 2880;               
    DEF << bdb_media_descriptor_type: BYTE:HEX << 0F0;                 
    DEF << bdb_sectors_per_ext: WORD << 9;                    
    DEF << bdb_sectors_per_track: WORD << 18;
    DEF << bdb_heads: WORD << 2;
    DEF << bdb_hidden_sectors: DOUBLE << 0;
    DEF << bdb_large_sector_count: DOUBLE << 0;

    if (DEV << FS() == "ext4") {                    // radi filesystem changes to ext2
        DEF << ext4_sectors_per_fat: DOUBLE << 0;
        DEF << ext4_flags: WORD << 0;
        DEF << ext4_fat_version_number: WORD << 0;
        DEF << ext4_rootdir_cluster: DOUBLE << 0;
        DEF << ext4_fsinfo_sector: WORD << 0;
        DEF << ext4_backup_boot_sector: WORD << 0;
        DEF << ext4_reserved: BYTES: TIMES 12 0;
    }
  }       
}
