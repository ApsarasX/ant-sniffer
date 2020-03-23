export function padHex(number: number, length: number = 4): string {
  const numHexStr = number.toString(16);
  const diffLen = length - numHexStr.length;
  return `0x${'0'.repeat(Math.max(diffLen, 0))}${numHexStr}`;
}

export function padFlag(index: number, flag: boolean, text: string): string {
  return `${' • '.repeat(index)}${flag ? '1' : '0'}${' • '.repeat(
    7 - index
  )} = ${text} ${flag ? '已' : '未'} 设置`;
}