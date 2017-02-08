import {observable, computed} from 'mobx';

export default class DestinationModel {
    @observable id: number;
    @observable type: string;
    @observable settings: any;

    constructor(id: number, type: string, settings: any) {
        this.id = id;
        this.type = type;
        this.settings = settings;
    }

    @computed get name() {
        return this.type;
    }

    @computed get asJson() {
        return {
            id: this.id,
            type: this.type,
            settings: this.settings
        }
    }
}