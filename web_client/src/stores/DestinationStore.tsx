import { observable, reaction } from 'mobx';
import {
    DestinationType,
    DestinationModel,
    DestinationSettings,
    TYPE_NULL,
    TYPE_DIRECTORY
} from '../models/DestinationModel';

export default class DestinationStore {
    @observable destinations: DestinationModel[] = [];

    constructor() {
        this.loadDestinations();
    }

    /**
     * Loads all destinations from the server
     */
    loadDestinations() {
        this.createDestination(TYPE_DIRECTORY, {
            path: "C:\\example"
        });
    }

    /**
     *  Creates a new destination
     */
    createDestination(type: DestinationType, settings: DestinationSettings) {
        this.destinations.push(new DestinationModel(Math.random(), type, settings));
    }
}