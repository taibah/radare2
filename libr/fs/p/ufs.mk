OBJ_UFS=fs_ufs.o
EXTRA=../p/grub/libgrubfs.a
CFLAGS+=-Igrub/include

STATIC_OBJ+=${OBJ_UFS}
#STATIC_OBJ+=${EXTRA}
TARGET_UFS=fs_ufs.${EXT_SO}

ALL_TARGETS+=${TARGET_UFS}

${TARGET_UFS}: ${OBJ_UFS}
	${CC} ${LDFLAGS} ${CFLAGS} -o ${TARGET_UFS} ${OBJ_UFS} ${EXTRA}