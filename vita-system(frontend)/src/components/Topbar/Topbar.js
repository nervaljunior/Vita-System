import React, { useState } from 'react';
import Navbar from '../Navbar/Navbar';
import styles from './Topbar.module.css';

function Topbar() {
  const [isNavbarOpen, setIsNavbarOpen] = useState(false);

  const toggleNavbar = () => {
    setIsNavbarOpen(!isNavbarOpen); 
  };

  return (
    <div styles={{}}>
      <nav className={styles.navbar}>
        <div className={styles.logo}>
          <h1>Vita System</h1>
          
        </div>
        
        <div className={styles.menu}>
          <a href="#">home</a>
          <a href="#">sobre nós</a>
          <a href="#">serviços</a>
          <a id="botao" href="#">fale conosco</a>
        </div>
      </nav>

      <div className={styles.row}>
        <div className="col-md-4">
          {/* Conteúdo da coluna */}
        </div>
      </div>

      <div className={styles.topbar}>
        <div className={styles.toggle} onClick={toggleNavbar}>
          <ion-icon name="menu-outline" />
        </div>
        {/* PROCURAR */}
        <div className={styles.search}>
          <label htmlFor="">
            <input type="text" placeholder="Digite o nome do paciente..." />
            <ion-icon name="search-outline" />
          </label>
        </div>
        {/* IMAGEM */}
        <button className={styles.user} onClick={toggleNavbar}>
          <img src="woman.png" />
        </button>
        {/* Renderize o Navbar com base no estado */}
        {isNavbarOpen && <Navbar />}
      </div>

    </div>
  );
}

export default Topbar;
