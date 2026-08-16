export * from './enums';

// tslint:disable-next-line:interface-name
export interface Hash<T> {
    [id: string]: T;
}
// tslint:disable-next-line:interface-name
export interface List<T> {
    [index: number]: T;
    length: number;
}
/**
 * Interface of the simple literal object with any string keys.
 */
export interface IObjectLiteral {
    [key: string]: any;
}
/**
 * Represents some Type of the Object.
 */
// tslint:disable-next-line:ban-types
export type ObjectType<T> = { new(): T } | (Function);
/**
 * Same as Partial<T> but goes deeper and makes Partial<T> all its properties and sub-properties.
 */
export type DeepPartial<T> = {
    [P in keyof T]?: DeepPartial<T[P]>;
};

export interface IDelimitter {
    begin: string;
    end: string;
}

export type JSONPathExpression = string;
