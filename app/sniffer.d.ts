export interface ISniffer {
  getDevs(): any[];
  openDev(
    devName: string,
    options?: {
      snaplen: number;
      promisc: boolean;
      bufSize: number;
      timeout: number;
    }
  ): ISnifferInstance;
}
export interface ISnifferInstance {
  setFilter(filter: string): boolean;
  start(): void;
  onProgress(cb: Function): void;
  keepAlive(): void;
  closeDev(): void;
}
