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
  }
  
}        

    .after:

        mov [ebr_drive_number], dl

        ; check extensions present
        mov ah, 0x41
        mov bx, 0x55AA
        stc
        int 13h

        jc .no_disk_extensions
        cmp bx, 0xAA55
        jne .no_disk_extensions

        ; extensions are present
        mov byte [have_extensions], 1
        jmp .after_disk_extensions_check

    .no_disk_extensions:
        mov byte [have_extensions], 0

    .after_disk_extensions_check:
        ; load stage2
        mov si, stage2_location

        mov ax, STAGE2_LOAD_SEGMENT         ; set segment registers
        mov es, ax

        mov bx, STAGE2_LOAD_OFFSET

    .loop:
        mov eax, [si]
        add si, 4
        mov cl, [si]
        inc si

        cmp eax, 0
        je .read_finish

        call disk_read

        xor ch, ch
        shl cx, 5
        mov di, es
        add di, cx
        mov es, di

        jmp .loop

    .read_finish:
        
        ; jump to our kernel
        mov dl, [ebr_drive_number]          ; boot device in dl
        mov si, PARTITION_ENTRY_OFFSET
        mov di, PARTITION_ENTRY_SEGMENT
    
        mov ax, STAGE2_LOAD_SEGMENT         ; set segment registers
        mov ds, ax
        mov es, ax

        jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

        jmp wait_key_and_reboot             ; should never happen

        cli                                 ; disable interrupts, this way CPU can't get out of "halt" state
        hlt


section .text

    ;
    ; Error handlers
    ;

    floppy_error:
        mov si, msg_read_failed
        call puts
        jmp wait_key_and_reboot

    kernel_not_found_error:
        mov si, msg_stage2_not_found
        call puts
        jmp wait_key_and_reboot

    wait_key_and_reboot:
        mov ah, 0
        int 16h                     ; wait for keypress
        jmp 0FFFFh:0                ; jump to beginning of BIOS, should reboot

    .halt:
        cli                         ; disable interrupts, this way CPU can't get out of "halt" state
        hlt


    ;
    ; Prints a string to the screen
    ; Params:
    ;   - ds:si points to string
    ;
    puts:
        ; save registers we will modify
        push si
        push ax
        push bx

    .loop:
        lodsb               ; loads next character in al
        or al, al           ; verify if next character is null?
        jz .done

        mov ah, 0x0E        ; call bios interrupt
        mov bh, 0           ; set page number to 0
        int 0x10

        jmp .loop

    .done:
        pop bx
        pop ax
        pop si    
        ret

    ;
    ; Disk routines
    ;

    ;
    ; Converts an LBA address to a CHS address
    ; Parameters:
    ;   - ax: LBA address
    ; Returns:
    ;   - cx [bits 0-5]: sector number
    ;   - cx [bits 6-15]: cylinder
    ;   - dh: head
    ;

    lba_to_chs:

        push ax
        push dx

        xor dx, dx                          ; dx = 0
        div word [bdb_sectors_per_track]    ; ax = LBA / SectorsPerTrack
                                            ; dx = LBA % SectorsPerTrack

        inc dx                              ; dx = (LBA % SectorsPerTrack + 1) = sector
        mov cx, dx                          ; cx = sector

        xor dx, dx                          ; dx = 0
        div word [bdb_heads]                ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                            ; dx = (LBA / SectorsPerTrack) % Heads = head
        mov dh, dl                          ; dh = head
        mov ch, al                          ; ch = cylinder (lower 8 bits)
        shl ah, 6
        or cl, ah                           ; put upper 2 bits of cylinder in CL

        pop ax
        mov dl, al                          ; restore DL
        pop ax
        ret


    ;
    ; Reads sectors from a disk
    ; Parameters:
    ;   - eax: LBA address
    ;   - cl: number of sectors to read (up to 128)
    ;   - dl: drive number
    ;   - es:bx: memory address where to store read data
    ;
    disk_read:

        push eax                            ; save registers we will modify
        push bx
        push cx
        push dx
        push si
        push di

        cmp byte [have_extensions], 1
        jne .no_disk_extensions

        ; with extensions
        mov [extensions_dap.lba], eax
        mov [extensions_dap.segment], es
        mov [extensions_dap.offset], bx
        mov [extensions_dap.count], cl

        mov ah, 0x42
        mov si, extensions_dap
        mov di, 3                           ; retry count
        jmp .retry

    .no_disk_extensions:
        push cx                             ; temporarily save CL (number of sectors to read)
        call lba_to_chs                     ; compute CHS
        pop ax                              ; AL = number of sectors to read
        
        mov ah, 02h
        mov di, 3                           ; retry count

    .retry:
        pusha                               ; save all registers, we don't know what bios modifies
        stc                                 ; set carry flag, some BIOS'es don't set it
        int 13h                             ; carry flag cleared = success
        jnc .done                           ; jump if carry not set

        ; read failed
        popa
        call disk_reset

        dec di
        test di, di
        jnz .retry

    .fail:
        ; all attempts are exhausted
        jmp floppy_error

    .done:
        popa

        pop di
        pop si
        pop dx
        pop cx
        pop bx
        pop eax                            ; restore registers modified
        ret


    ;
    ; Resets disk controller
    ; Parameters:
    ;   dl: drive number
    ;
    disk_reset:
        pusha
        mov ah, 0
        stc
        int 13h
        jc floppy_error
        popa
        ret

section .rodata

    msg_read_failed:        db 'Read failed!', ENDL, 0
    msg_stage2_not_found:   db 'STAGE2.BIN not found!', ENDL, 0
    file_stage2_bin:        db 'STAGE2  BIN'

section .data

    have_extensions:        db 0
    extensions_dap:
        .size:              db 10h
                            db 0
        .count:             dw 0
        .offset:            dw 0
        .segment:           dw 0
        .lba:               dq 0

    STAGE2_LOAD_SEGMENT     equ 0x0
    STAGE2_LOAD_OFFSET      equ 0x500

    PARTITION_ENTRY_SEGMENT equ 0x2000
    PARTITION_ENTRY_OFFSET  equ 0x0


section .data
    global stage2_location
    stage2_location:        times 30 db 0

section .bss
    buffer:                 resb 512
