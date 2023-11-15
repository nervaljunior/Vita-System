import React, { useState } from 'react';
import Navbar from '../Navbar/Navbar';
import styles from './Topbar.module.css';

 

function Topbar() {
  const [isMenuOpen, setMenuOpen] = useState(false);
  const [isNavbarOpen, setIsNavbarOpen] = useState(false);
  
  const toggleMenu = () => {
    setMenuOpen(!isMenuOpen);
  };
   
   const toggleNavbar = () => {
     setIsNavbarOpen(!isNavbarOpen); 
  };

  return (
    
      <>
        <nav className={styles.navbar}>
          <div className={styles.logo}>

            <h1>Vita System</h1>

          </div>    
          <div className={styles.menu}>
            <a href="/">Início</a>
            <a href="#">Sobre nós</a>
            <a href="#">Projetos</a>
            <a id="botao" href="#">Fale conosco</a>
          </div>

          <div className={styles.topbar}>
            <div className={styles.loginButton}>
              <button><a href="/login">Entrar</a></button>
            </div>
          <div className={styles.toggle} onClick={toggleNavbar}>
            <ion-icon name="menu-outline" />
          </div>

          {isNavbarOpen && <Navbar />}
        </div>
        </nav>

        <div className={styles.mobileMenuIcon}>
            <button onClick={toggleMenu}><img className="icon" src="menu_white_36dp.svg" alt="" /></button>
          </div>
          <div className={styles.mobileMenu} open={isMenuOpen}>
            <ul className={styles.menu}>
              <li className={styles.navItem}><a className={styles.navLink} href="#">Início</a></li>
              <li className={styles.navItem}><a className={styles.navLink} href="#">Projetos</a></li>
              <li className={styles.navItem}><a className={styles.navLink} href="#">Sobre nós</a></li>
              <li className={styles.navItem}><a className={styles.navLink} href="#">Serviços</a></li>
              <li className={styles.navItem}><a className={styles.navLink} href="#">Fale conosco</a></li>
            </ul>
            <div className={styles.loginButton}>
              <button><a href="#">Entrar</a></button>
            </div>
          </div>
  </>
  );
}

export default Topbar;

/*

<div className={styles.search}>
  <label htmlFor="">
  <input type="text" placeholder="Digite o nome do paciente..." />
    <ion-icon name="search-outline" />
  </label>
</div> 


*/



{/*         <button className={styles.user} onClick={toggleNavbar}>
          <img src="woman.png" />
        </button> */}