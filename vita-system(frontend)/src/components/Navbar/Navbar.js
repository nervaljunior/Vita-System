// Navbar.js
import React from 'react';
import './Navbar.module.css'

function Navbar() {
  return (
    <>
      {/* Conteúdo da barra de navegação */}
      <div className="navegation">
      <ul>
        <li>
          <a href="#">
            <span className="icon">
              <ion-icon name="medical-outline" />
            </span>
            <span className="tittle">PROJETO VITA</span>
          </a>
        </li>
        <li>
          <a href="#">
            <span className="icon">
              <ion-icon name="help-outline" />
            </span>
            <span className="tittle">Ajuda</span>
          </a>
        </li>
        <li>
          <a href="#">
            <span className="icon">
              <ion-icon name="construct-outline" />
            </span>
            <span className="tittle">Configurações</span>
          </a>
        </li>
        <li>
          <a href="#">
            <span className="icon">
              <ion-icon name="lock-closed-outline" />
            </span>
            <span className="tittle">Senha</span>
          </a>
        </li>
        <li>
          <a href="#">
            <span className="icon">
              <ion-icon name="log-out-outline" />
            </span>
            <span className="tittle">Sign Out</span>
          </a>
        </li>
      </ul>
    </div>
    </>

  );
}

export default Navbar;
