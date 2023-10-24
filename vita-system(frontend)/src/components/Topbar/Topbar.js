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

        <div className={styles.topbar}>
        <div className={styles.toggle} onClick={toggleNavbar}>
          <ion-icon name="menu-outline" />
        </div>

        <button className={styles.user} onClick={toggleNavbar}>
          <img src="woman.png" />
        </button>
        {isNavbarOpen && <Navbar />}
      </div>
      </nav>




    </div>
  );
}

export default Topbar;

{/*         <div className={styles.search}>
          <label htmlFor="">
            <input type="text" placeholder="Digite o nome do paciente..." />
            <ion-icon name="search-outline" />
          </label>
        </div> */}