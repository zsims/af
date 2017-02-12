import {observable, computed} from 'mobx';

export const TYPE_NULL = "null";
export const TYPE_DIRECTORY = "directory";

export type DestinationType = "null" | "directory";
export type DestinationSettings = { [key: string]: any };

export class DestinationModel  {
    @observable id: number;
    type: DestinationType;
    @observable settings: DestinationSettings;

    constructor(id: number, type: DestinationType, settings: DestinationSettings) {
        this.id = id;
        this.type = type;
        this.settings = settings;
    }
}
