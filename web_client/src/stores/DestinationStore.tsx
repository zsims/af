import { observable, reaction } from 'mobx';
import DestinationModel from '../models/DestinationModel';
import * as DestinationType from '../models/DestinationType';

export default class DestinationStore {
    @observable destinations: DestinationModel[] = [];

    constructor() {
        this.loadDestinations();
    }

    /**
     * Loads all destinations from the server
     */
    loadDestinations() {
        this.createDestination(DestinationType.NULL, {});
        this.createDestination(DestinationType.DIRECTORY, {
            path: "C:\\here"
        });
    }

    /**
     *  Creates a new destination
     */
    createDestination(type: string, settings: Object) {
        this.destinations.push(new DestinationModel(Math.random(), type, settings));
    }
}