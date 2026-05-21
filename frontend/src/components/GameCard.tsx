import React, { useState } from 'react';
import { Card } from 'antd';
import { Game } from '../types';

interface GameCardProps {
  game: Game;
  onClick?: (game: Game) => void;
}

const GameCard: React.FC<GameCardProps> = ({ game, onClick }) => {
  const [imgError, setImgError] = useState(false);

  return (
    <Card
      hoverable
      className="cursor-pointer transition-all duration-300 hover:shadow-lg"
      cover={
        <div className="h-40 flex items-center justify-center bg-gradient-to-br from-blue-50 to-blue-100 overflow-hidden">
          {game.image && !imgError ? (
            <img
              src={game.image}
              alt={game.name}
              className="w-full h-full object-cover"
              onError={() => setImgError(true)}
            />
          ) : (
            <div className="text-5xl">{game.icon}</div>
          )}
        </div>
      }
      onClick={() => onClick?.(game)}
    >
      <Card.Meta title={game.name} className="text-center" />
    </Card>
  );
};

export default GameCard;
