package com.example.appalimentadormascotas.Models;

import android.graphics.Bitmap;

public class Image {
    private String imageName;
    private Bitmap image;

    public Image(String imageName, Bitmap image) {
        this.imageName = imageName;
        this.image = image;
    }

    public Image() {

    }

    public String getImageName() {
        return imageName;
    }

    public void setImageName(String imageName) {
        this.imageName = imageName;
    }

    public Bitmap getImage() {
        return image;
    }

    public void setImage(Bitmap image) {
        this.image = image;
    }


}
