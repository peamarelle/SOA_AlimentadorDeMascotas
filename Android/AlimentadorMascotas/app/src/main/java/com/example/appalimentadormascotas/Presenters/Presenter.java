package com.example.appalimentadormascotas.Presenters;

import android.content.Context;

import com.example.appalimentadormascotas.Contracts.Contract;
import com.example.appalimentadormascotas.Models.Image;
import com.example.appalimentadormascotas.Models.Mascota;

public class Presenter implements Contract.Presenter, Contract.Model.OnEventListener{
    private Contract.View mainView;
    private Contract.Model model;



    public Presenter (Contract.View mainView, Contract.Model model){
        this.mainView = mainView;
        this.model = model;
    }
    @Override
    public void onSaveButtonClick(Mascota mascota, Image image) {
        model.saveMascota(mascota,image,mainView,this);
    }



    @Override
    public void onImgButtonClick() {

    }

    @Override
    public void onToAppButtonClick() {

    }

    @Override
    public void onLightSensorChanged(Float sensorValue) {

    }


    @Override
    public void onDestroy() {
        mainView = null;
    }

    @Override
    public void onCreate() {

    }

    @Override
    public void onEvent(String string, int evento){
        switch(evento) {
            case Contract.SAVE_MASCOTA_EVENT:
                mainView.setString(string);
                break;
            case Contract.SAVE_IMG_EVENT:
                System.out.println("i es 2.");
                break;
            case Contract.TO_APP_EVENT:
                System.out.println("i es 3.");
                break;
            default:
                throw new IllegalStateException("Unexpected value: " + evento);
        }

    }

}