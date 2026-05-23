#!/bin/bash

set -e

VG=${VALGRIND_CMD:-valgrind --error-exitcode=1 --leak-check=full -q}

echo "=== Подготовка образа ==="

truncate --size 5G ext2.img

mkfs.ext2 -F -b 2048 -I 256 ext2.img

mkdir -p ext2_mnt
sudo mount -t ext2 ext2.img ext2_mnt

sudo chown $(whoami):$(whoami) ext2_mnt

echo "=== Генерация файлов внутри ФС ==="

mkdir -p ext2_mnt/dir1/subdir
mkdir -p ext2_mnt/dir2

echo "Генерация 10MB файла (непрямая адресация)..."
dd if=/dev/urandom of=ext2_mnt/large_file bs=1M count=10 status=none

echo "Генерация 4.1GB разреженного файла..."
dd if=/dev/zero of=ext2_mnt/sparse_file bs=1 count=0 seek=4400000000 status=none
echo "Some data at the end of a very large sparse file" >> ext2_mnt/sparse_file

INODE_LARGE=$(stat -c %i ext2_mnt/large_file)
INODE_SPARSE=$(stat -c %i ext2_mnt/sparse_file)
INODE_DIR=$(stat -c %i ext2_mnt/dir1)

HASH_LARGE=$(sha512sum ext2_mnt/large_file | awk '{print $1}')
HASH_SPARSE=$(sha512sum ext2_mnt/sparse_file | awk '{print $1}')

echo "Сохраненные метаданные для проверки:"
echo "large_file:  Inode=$INODE_LARGE, SHA512=$HASH_LARGE"
echo "sparse_file: Inode=$INODE_SPARSE, SHA512=$HASH_SPARSE"
echo "dir1:        Inode=$INODE_DIR"

sudo umount ext2_mnt

echo "=== Тестирование утилит (Обычный файл-образ) ==="

echo -e "\n--- [Тест 1] Метаданные large_file (ext2_info) ---"
$VG ./ext2_info ext2.img $INODE_LARGE

echo -e "\n--- [Тест 2] Извлечение данных (ext2_cat) ---"
echo "Сверка контрольной суммы для large_file (с непрямой адресацией)..."
TEST_HASH_LARGE=$($VG ./ext2_cat ext2.img $INODE_LARGE | sha512sum | awk '{print $1}')
if [ "$HASH_LARGE" = "$TEST_HASH_LARGE" ]; then
    echo "[OK] Хеши large_file совпали!"
else
    echo "[FAIL] ОШИБКА: Хеши large_file различаются!"
    exit 1
fi

echo "Сверка контрольной суммы для sparse_file (>4G, разреженный)..."
TEST_HASH_SPARSE=$($VG ./ext2_cat ext2.img $INODE_SPARSE | sha512sum | awk '{print $1}')
if [ "$HASH_SPARSE" = "$TEST_HASH_SPARSE" ]; then
    echo "[OK] Хеши sparse_file совпали!"
else
    echo "[FAIL] ОШИБКА: Хеши sparse_file различаются!"
    exit 1
fi

echo -e "\n--- [Тест 3] Содержимое корневого каталога (ext2_ls) ---"
./ext2_cat ext2.img 2 | $VG ./ext2_ls > ls_output.txt
cat ls_output.txt

echo "Сверка Inode извлеченного каталога dir1 с эталоном..."
EXTRACTED_DIR_INODE=$(grep "dir1$" ls_output.txt | awk '{print $1}')
if [ "$INODE_DIR" = "$EXTRACTED_DIR_INODE" ]; then
    echo "[OK] Папка 'dir1' найдена, Inode совпадает с эталонным ($INODE_DIR)!"
else
    echo "[FAIL] ОШИБКА: Неверный Inode или папка не найдена в каталоге!"
    exit 1
fi


echo -e "\n=== Тестирование на Loop-устройстве ==="

LOOP_DEV=$(sudo losetup -f)
echo "Найден свободный loop-интерфейс: $LOOP_DEV"

sudo losetup $LOOP_DEV ext2.img

sudo chmod a+r $LOOP_DEV

echo "Вывод команды losetup -a:"
losetup -a | grep ext2.img

echo "Информация от lsblk (имя, размер, тип ФС):"
lsblk -o name,size,fstype $LOOP_DEV

echo "Сверка хеша large_file при чтении из блочного устройства $LOOP_DEV..."
LOOP_HASH_LARGE=$($VG ./ext2_cat $LOOP_DEV $INODE_LARGE | sha512sum | awk '{print $1}')
if [ "$HASH_LARGE" = "$LOOP_HASH_LARGE" ]; then
    echo "[OK] Хеши совпали! Утилиты корректно работают с блочными устройствами POSIX."
else
    echo "[FAIL] ОШИБКА: Хеши при чтении через loop-девайс различаются!"
    sudo losetup -d $LOOP_DEV
    exit 1
fi

sudo losetup -d $LOOP_DEV

echo -e "\n=== Задание успешно выполнено в полном объеме! ==="