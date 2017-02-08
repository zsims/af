import {observable, computed} from 'mobx';

export const TYPE_NULL = "null";
export const TYPE_DIRECTORY = "directory";

export interface IDestinationModel {
    id: number;
    type: string;
    readonly name: string;
    readonly summary: string;
    readonly asJson: any;
}

export class DirectoryDestinationModel implements IDestinationModel {
    @observable id: number;
    type = TYPE_DIRECTORY;
    @observable settings: {
        path: string
    } = {
        path: ""
    };

    get name(): string {
        return "Directory";
    }

    get summary(): string {
        return this.settings.path;
    }

    get asJson(): any {
        return {
            id: this.id,
            type: this.type,
            settings: {
                path: this.settings.path
            }
        };
    }
}

export class NullDestinationModel implements IDestinationModel {
    @observable id: number;
    type = TYPE_NULL;

    get name(): string {
        return "Null";
    }

    get summary(): string {
        return "";
    }

    get asJson(): any {
        return {
            id: this.id,
            type: this.type,
            settings: {}
        };
    }
}