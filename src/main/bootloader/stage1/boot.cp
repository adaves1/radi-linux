// C+ Version 22
// crazy that c+ is inspired by java, c, c++, c#, python

package "radi.linux.pkg" with ".stage1.boot";

#DEFINE ENDL "0x0D" and "0x0A";

#DEINE fat12 "1";
#DEFINE fat16 "2";
#DEFINE fat32 "3";
#DEFINE ext2 "4";

CPLUS NAMESPACE_USGAE = ["MEM", "BYTE", "WORD", "DOUBLE", "DEF", "DEV", "HEX"];
CPLUS NAMESPACE_USAGE_AUTO_ASSIGN_TYPE = "auto-NAMESPACE_USAGE-toall-wmarks";

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

  // Extended Boot Record

  DEF << ebr_drive_number: BYTE << 0;
  DEF << BYTE << 0;                                   // Reserved
  DEF << ebr_signature << BYTE:HEX << 29;
  DEF << ebr_volume_id: BYTE:HEX << 12, 34, 56, 78;
  DEF << ebr_volume_label: BYTE << "Radi Linux";
  DEF << ebr_system_id: BYTE << "ext2";
  
  }       
}

class .entry {
  loop start;

  l:start {
    DEF << move REG ax : DEV << PARTITION_ENTRY_OFFSET;
    DEF << move REG es : REG ax;
    DEF << move REG di : DEV << PARTITION_ENTRY_OFFSET;
    DEF << move REG cx : 16;
    DEF << rept DEV << movsb;
    DEF << move REG ax : 0;
    DEF << move REG ds : REG ax;
    DEF << move REG es : REG ax;
    DEF << move REG ss : REG ax;
    DEF << move REG sp : SPC 0x7C00;
    DEF << push REG es;
    DEF << push WORD:after;
    DEF << retf;

    p:w:after {
      DEF << move ebr_drive_number : REG dl;
      DEF << move REG ah : SPC 0x41;
      DEF << move REG bx : 0x55AA;
      DEF << stc:ef;
      DEF << intg HEX:13;

      DEF << jumpc .no_disk_extensions;
      DEF << cmp REG bx : SPC 0xAA55;
      DEF << jumpnec .no_disk_extensions;

      DEF << move BYTE:have_extensions : 1;
      DEF << JUMP .after_disk_extensions_check;

      class .no_disks_extensions {
        DEF << move BYTE:have_extensions : 0;
        
      }

      class .after_disk_extensions_check {
        DEF << move REG si : stage2_location;        // Remember that when a variable is non-existent, c+ creates it

        DEF << move REG ax : STAGE2_LOAD_SEGMENT;
        DEF << move REG es : REG ax;

        DEF << move REG bx : STAGE2_LOAD_OFFSET;
      }

      loop .loop;
      l:.loop {
        DEF << move REG eax : REG si;
        DEF << add REG si : 4;
        DEF << move REG cl : REG si;
        DEF << incl REG si;

        DEF << cmp REG eax : 0;

        DEF << jetfs .read_finish;

        DEF << call disk_read;
        DEF << xor REG ch : REG ch;
        DEF << shal REG cx : 5;
        DEF << move REG di : REG es;
        DEF << move REG di : REG cx;
        DEF << move REG es : REG di;
      }

      loop .read_finish;
      l:.read_finish {
        DEF << move REG dl : ebr_drive_number;
        DEF << move REG si : PARTITION_ENTRY_OFFSET;
        DEF << move REG di : PARTITION_ENTRY_SEGMENT;

        DEF << move REG ax : STAGE2_LOAD_SEGMENT;
        DEF << move REG ds : REG ax;
        DEF << move REG es : REG ax;
        DEF << JUMP STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET;
        DEF << JUMP wait_key_and_reboot;
        DEF << DEVICE DISABLE_INTERRUPTS = "1";
        DEF << DEVICE halt;
      }
    }
  }
}        

class .text {
  loop floppy_error;
  l:floppy_error {
    DEF << move REG si : msg_read_failed;
    DEF << call puts;
    DEF << JUMP wait_key_and_reboot;
  }

  loop kernel_not_found_error;
  l:kernel_not_found_error {
    DEF << move REG si : msg_stage2_not_found;
    DEF << call puts;
    DEF << JUMP wait_key_and_reboot;
  }

  loop wait_key_and_reboot;
  l:wait_key_and_reboot {
    DEF << move REG ah : 0;
    DEF << intg HEX:16;
    DEF << JUMP HEX:0FFFF:0;
  }

  DEF << .halt extends {
    DEF << DEVICE_DISABLE_INTERRUPTS = "1";
    DEF << DEVICE halt;
  }

  DEF << puts extends {
    DEF << push REG si;
    DEF << push REG ax;
    DEF << push REG bx;
  }

  l:.loop extends {
    DEF << loadnchar or REG al : REG al;
    DEF << jzmove .done;

    DEF << move REG ah : SPC 0x0E;
    DEF << move REG bh : 0;
    DEF << intg SPC 0x10;
  }

  loop .done;
  l:.done {
    DEF << pop REG bx;
    DEF << pop REG ax;
    DEF << pop REG si;
    DEF << retrn;
  }

  loop lba_to_chs;
  l:lba_to_chs {
    DEF << push REG ax;
    DEF << push REG dx;

    DEF << xor REG dx : REG dx;
    DEF << div word bdb_sectors_per_track;

    DEF << incl REG dx;
    DEF << move REG cx : REG dx;

    DEF << xor REG dx : REG dx;
    DEF << div word bdb_heads;

    DEF << move REG dh : REG dl;
    DEF << move REG ch : REG al;
    DEF << shal REG ah : 6 : cl : ah;

    DEF << pop REG ax;
    DEF << move REG dl : REG al;
    DEF << pop REG ax;
    DEF << retrn;
  }

  loop disk_read;
  l:disk_read {
    DEF << push REG eax;
    DEF << push REG bx;
    DEF << push REG cx;
    DEF << push REG dx;
    DEF << push REG si;
    DEF << push REG di;

    DEF << cmp BYTE have_extensions : 1;
    DEF << jetfs .no_disk_extensions;

    DEF << move extensions_dap.lba : REG eax;
    DEF << move extensions_dap.segment : REG es;
    DEF << move extensions_dap.offset: REG bx;
    DEF << move extensions_dap.count : REG cl;

    DEF << move REG ah : SPC 0x42;
    DEF << move REG si : extensions_dap;
    DEF << move REG di : 3;
    DEF << JUMP .retry;
    
  }

  l:.no_disk_extensions extends {
    DEF << push REG cx;
    DEF << call lba_to_chs;
    DEF << pop REG ax;

    DEF << move REG ah : HEX:02;
    DEF << move REG di : 3;
  }

  loop .retry;
  l:.retry {
    DEF << push all;
    DEF << stc:ef;
    DEF << intg HEX:13;
    DEF << jumpnec .done;

    DEF << pop all;
    DEF << call disk_reset;

    DEF << dec REG di;
    DEF << UNITTEST REG di : REG di;
    DEF << jetfsz .retry;
  }

  loop .fail;
  l:.fail {
    DEF << JUMP floppy_error;
  }

  loop .done;
  l:.done {
    DEF << pop all;
    DEF << pop REG di;
    DEF << pop REG si;
    DEF << pop REG dx;
    DEF << pop REG cx;
    DEF << pop REG bx;
    DEF << pop REG eax;
    DEF << retrn;
  }

  loop disk_reset;
  l:disk_reset {
    DEF << push all;
    DEF << move REG ah : 0;
    DEF << stc:ef;
    DEF << intg HEX:13;
    DEF << jumpc floppy_error;
    DEF << pop all;
    DEF << retrn;
  }
}

class .rodata {
  msg_read_failed = "Read failed!", ENDL, 0;
  msg_stage2_not_found = "STAGE2.BIN not found!", ENDL, 0;
  file_stage2_bin = "STAGE2  BIN";
}

class .data {
  have_extensions = 0;
  
  object extensions_dap;
  extensions_dap.size = HEX:10; BYTE:0;
  extensions_dap.count = 0;
  extensions_dap.offset = 0;
  extensions_dap.segment = 0;
  extensions_dap.lba = 0;

  DEF << global STAGE2_LOAD_SEGMENT = equ SPC 0x0;
  DEF << global STAGE2_LOAD_OFFSET = equ SPC 0x500;

  DEF << global PARTITION_ENTRY_SEGMENT = equ SPC 0x2000;
  DEF << global PARTITION_ENTRY_OFFSET = equ SPC 0x0;
}


class .data extends {
  global stage2_location;
  stage2_location = DEF << TIMES 30 BYTE:0;
}

class .bss {
  buffer = DEF << resb 512;
}
