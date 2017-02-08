import { observable, reaction } from 'mobx';
import {IDestinationModel, NullDestinationModel, DirectoryDestinationModel} from '../models/DestinationModel';

export default class DestinationStore {
    @observable destinations: IDestinationModel[] = [];

    constructor() {
        this.loadDestinations();
    }

    /**
     * Loads all destinations from the server
     */
    loadDestinations() {
        this.createDestination(new NullDestinationModel());
        this.createDestination(new DirectoryDestinationModel());
    }

    /**
     *  Creates a new destination
     */
    createDestination(destination: IDestinationModel) {
        destination.id = Math.random();
        this.destinations.push(destination);
    }
}