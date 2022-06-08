/// <reference types="node" />
export declare function register(windowHandle: Buffer, emit: (event: string | symbol, ...args: any[]) => boolean): any;
export declare function dock(windowHandle: Buffer, side:boolean, left: number, top: number, width: number, height: number): any;
export declare function unregister(windowHandle: Buffer): any;
