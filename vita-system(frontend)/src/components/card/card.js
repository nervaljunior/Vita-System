// Card.js
import React from 'react';

function Card({ number, cardName, iconName }) {
  return (
    <div className="card">
      <div>
        <div className="numbers">{number}</div>
        <div className="cardName">{cardName}</div>
      </div>
      <div className="iconBox">
        <ion-icon name={iconName} />
      </div>
    </div>
  );
}

export default Card;
