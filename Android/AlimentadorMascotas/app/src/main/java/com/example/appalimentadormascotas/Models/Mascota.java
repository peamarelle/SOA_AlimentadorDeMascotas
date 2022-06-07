package com.example.appalimentadormascotas.Models;

import com.example.appalimentadormascotas.Contracts.Contract;

public class Mascota {

    private int idMascota;
    private String nombre;
    private int edad;
    private String Tipo;
    private int peso;

    public Mascota(int idMascota, String nombre, int edad, String tipo, int peso)  {
        this.idMascota = idMascota;
        this.nombre = nombre;
        this.edad = edad;
        Tipo = tipo;
        this.peso = peso;
    }

    public Mascota() {

    }

    public int getIdMascota() {
        return idMascota;
    }

    public void setIdMascota(int idMascota) {
        this.idMascota = idMascota;
    }

    public String getNombre() {
        return nombre;
    }

    public void setNombre(String nombre) {
        this.nombre = nombre;
    }

    public int getEdad() {
        return edad;
    }

    public void setEdad(int edad) {
        this.edad = edad;
    }

    public String getTipo() {
        return Tipo;
    }

    public void setTipo(String tipo) {
        Tipo = tipo;
    }

    public int getPeso() {
        return peso;
    }

    public void setPeso(int peso) {
        this.peso = peso;
    }

    @Override
    public String toString() {
        return "MascotaModel{" +
                "idMascota=" + idMascota +
                ", nombre='" + nombre + '\'' +
                ", edad=" + edad +
                ", Tipo='" + Tipo + '\'' +
                ", peso=" + peso +
                '}';
    }

}
