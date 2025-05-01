#!/bin/bash

# 定义 zip 文件名
ZIP_NAME="submit.zip"

# 需要排除的文件/文件夹
EXCLUDE_LIST=(
    ".git"
    "irsim"
    "*.o"
)

# 组合排除参数
EXCLUDE_ARGS=()
for item in "${EXCLUDE_LIST[@]}"; do
    EXCLUDE_ARGS+=("-x" "$item/*" "-x" "$item")
done

echo "Remove zip"
rm *.zip

# 执行 zip 命令
zip -r "$ZIP_NAME" ./* "${EXCLUDE_ARGS[@]}"

# 检查是否成功
if [ $? -eq 0 ]; then
    echo "打包成功: $ZIP_NAME"
else
    echo "打包失败！"
    exit 1
fi