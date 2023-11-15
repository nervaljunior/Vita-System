// components/HomePage.js
import React from 'react';
import styles from './HomePage.module.css';

const HomePage = () => {
  return (
    
    <div className={styles.container}>
      <header>
        <h1>VITA – VENTILAÇÃO INTENSIVA TECNOLOGICAMENTE ASSISTIDA</h1>
      </header>
      <main>
        <article>
          <p>
            Ainda a espera do que irá acontecer em termos de resposta da capacidade do sistema de saúde
            brasileiro (Capacidade SUS + hospitais privados) para atender a pandemia do COVID-19 no Brasil,
            diferentes iniciativas surgiram como resposta a possibilidade de aumento desta capacidade para
            atender às necessidades impostas pela pandemia. ...
          </p>
        </article>
        <aside className={styles.aside}>
          <h2>Conteúdo Relacionado</h2>
          <ul>
            <li><a href="#" className={styles.link}>Link 1</a></li>
            <li><a href="#" className={styles.link}>Link 2</a></li>
            <li><a href="#" className={styles.link}>Link 3</a></li>
          </ul>
        </aside>
      </main>
      <footer className={styles.footer}>
        <p>
          &copy; 2023 VITA – VENTILAÇÃO INTENSIVA TECNOLOGICAMENTE ASSISTIDA | Desenvolvido por [Seu Nome].
        </p>
      </footer>
    </div>
  );
};

export default HomePage;
