/**
 * Copyright (c) 2017 - present Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

type t 'a = {
  left: array 'a,
  middle: IndexedTrie.t 'a,
  right: array 'a,
};

include (Indexed.Make1 {
  type nonrec t 'a = t 'a;

  let count ({ left, middle, right }: t 'a): int => {
    let leftCount = CopyOnWriteArray.count left;
    let middleCount = IndexedTrie.count middle;
    let rightCount = CopyOnWriteArray.count right;

    leftCount + middleCount + rightCount;
  };

  let getOrRaise (index: int) ({ left, middle, right }: t 'a): 'a => {
    let leftCount = CopyOnWriteArray.count left;
    let middleCount = IndexedTrie.count middle;

    let rightIndex = index - middleCount - leftCount;

    if (index < leftCount) left.(index)
    else if (rightIndex >= 0) right.(rightIndex)
    else {
      let index = index - leftCount;
      middle |> IndexedTrie.get index;
    }
  };

  let reduceImpl (f: 'acc => 'a => 'acc) (acc: 'acc) ({ left, middle, right }: t 'a): 'acc => {
    let acc = left |> CopyOnWriteArray.reduce f acc;
    let acc = middle |> IndexedTrie.reduce f acc;
    let acc = right |> CopyOnWriteArray.reduce f acc;
    acc;
  };

  let reduceWhile
      while_::(predicate: 'acc => 'a => bool)
      (f: 'acc => 'a => 'acc)
      (acc: 'acc)
      ({ left, middle, right }: t 'a): 'acc => {
    let shouldContinue = ref true;
    let predicate acc next => {
      let result = predicate acc next;
      shouldContinue := result;
      result;
    };

    let triePredicate _ _ => !shouldContinue;
    let rec trieReducer acc =>
      IndexedTrie.reduceWhileWithResult triePredicate trieReducer predicate f acc;

    let acc = left |> CopyOnWriteArray.reduce while_::predicate f acc;

    let acc =
      if (!shouldContinue) (IndexedTrie.reduceWhileWithResult
        triePredicate
        trieReducer
        predicate
        f
        acc
        middle
      )
      else acc;

    let acc =
      if (!shouldContinue) (CopyOnWriteArray.reduce while_::predicate f acc right)
      else acc;

    acc;
  };

  let reduce
      while_::(predicate: 'acc => 'a => bool)
      (f: 'acc => 'a => 'acc)
      (acc: 'acc)
      (vec: t 'a): 'acc =>
    if (predicate === Functions.alwaysTrue2) (reduceImpl f acc vec)
    else (reduceWhile while_::predicate f acc vec);

  let reduceReversedImpl (f: 'acc => 'a => 'acc) (acc: 'acc) ({ left, middle, right }: t 'a): 'acc => {
    let acc = right |> CopyOnWriteArray.reduceReversed f acc;
    let acc = middle |> IndexedTrie.reduceReversed f acc;
    let acc = left |> CopyOnWriteArray.reduceReversed f acc;
    acc;
  };

  let reduceReversedWhile
      while_::(predicate: 'acc => 'a => bool)
      (f: 'acc => 'a => 'acc)
      (acc: 'acc)
      ({ left, middle, right }: t 'a): 'acc => {
    let shouldContinue = ref true;
    let predicate acc next => {
      let result = predicate acc next;
      shouldContinue := result;
      result;
    };

    let triePredicate _ _ => !shouldContinue;
    let rec trieReducer acc =>
      IndexedTrie.reduceReversedWhileWithResult triePredicate trieReducer predicate f acc;

    let acc = right |> CopyOnWriteArray.reduceReversed while_::predicate f acc;

    let acc =
      if (!shouldContinue) (IndexedTrie.reduceReversedWhileWithResult
        triePredicate
        trieReducer
        predicate
        f
        acc
        middle
      )
      else acc;

    let acc =
      if (!shouldContinue) (CopyOnWriteArray.reduceReversed while_::predicate f acc left)
      else acc;

    acc;
  };

  let reduceReversed
      while_::(predicate: 'acc => 'a => bool)
      (f: 'acc => 'a => 'acc)
      (acc: 'acc)
      (vec: t 'a): 'acc =>
    if (predicate === Functions.alwaysTrue2) (reduceReversedImpl f acc vec)
    else (reduceReversedWhile while_::predicate f acc vec);

  let toSequence ({ left, middle, right }: t 'a): (Sequence.t 'a) => Sequence.concat [
    CopyOnWriteArray.toSequence left,
    IndexedTrie.toSequence middle,
    CopyOnWriteArray.toSequence right,
  ];

  let toSequenceReversed ({ left, middle, right }: t 'a): (Sequence.t 'a) => Sequence.concat [
    CopyOnWriteArray.toSequenceReversed right,
    IndexedTrie.toSequenceReversed middle,
    CopyOnWriteArray.toSequenceReversed left,
  ];
}: Indexed.S1 with type t 'a := t 'a);

let emptyInstance: t 'a = {
  left: [||],
  middle: IndexedTrie.empty,
  right: [||],
};

let empty (): t 'a => emptyInstance;

let tailIsFull (arr: array 'a): bool =>
  (CopyOnWriteArray.count arr) === IndexedTrie.width;

let tailIsNotFull (arr: array 'a): bool =>
  (CopyOnWriteArray.count arr) !== IndexedTrie.width;

let addFirst (value: 'a) ({ left, middle, right }: t 'a): (t 'a) =>
  if ((tailIsFull left) && (CopyOnWriteArray.isNotEmpty right)) {
    left: [| value |],
    middle: IndexedTrie.addFirstLeafUsingMutator IndexedTrie.updateLevelPersistent Transient.Owner.none left middle,
    right,
  }
  else if ((tailIsFull left) && (CopyOnWriteArray.isEmpty right)) {
    left: [| value |],
    middle,
    right: left,
  }
  else {
    left: left |> CopyOnWriteArray.addFirst value,
    middle,
    right,
  };

let addLast (value: 'a) ({ left, middle, right }: t 'a): (t 'a) =>
  /* If right is empty, then middle is also empty */
  if ((tailIsNotFull left) && (CopyOnWriteArray.isEmpty right)) {
    left: left |> CopyOnWriteArray.addLast value,
    middle,
    right,
  }
  else if (tailIsNotFull right) {
    left,
    middle,
    right: right |> CopyOnWriteArray.addLast value,
  }
  else {
    left,
    middle: IndexedTrie.addLastLeafUsingMutator IndexedTrie.updateLevelPersistent Transient.Owner.none right middle,
    right: [| value |],
  };

let removeAll (_: t 'a): (t 'a) => emptyInstance;

let removeFirstOrRaise ({ left, middle, right }: t 'a): (t 'a) => {
  let leftCount = CopyOnWriteArray.count left;
  let middleCount = IndexedTrie.count middle;
  let rightCount = CopyOnWriteArray.count right;

  if (leftCount > 1) {
    left: CopyOnWriteArray.removeFirstOrRaise left,
    middle,
    right,
  }
  else if (middleCount > 0) {
    let (IndexedTrie.Leaf _ left, middle) =
      IndexedTrie.removeFirstLeafUsingMutator IndexedTrie.updateLevelPersistent Transient.Owner.none middle;
    { left, middle, right };
  }
  else if (rightCount > 0) {
    left: right,
    middle,
    right: [||],
  }
  else if (leftCount === 1) (empty ())
  else failwith "vector is empty";
};

let removeLastOrRaise ({ left, middle, right }: t 'a): (t 'a) => {
  let leftCount = CopyOnWriteArray.count left;
  let middleCount = IndexedTrie.count middle;
  let rightCount = CopyOnWriteArray.count right;

    if (rightCount > 1) {
      left,
      middle,
      right: CopyOnWriteArray.removeLastOrRaise right,
    }
    else if (middleCount > 0) {
      let (middle, IndexedTrie.Leaf _ right) =
        IndexedTrie.removeLastLeafUsingMutator IndexedTrie.updateLevelPersistent Transient.Owner.none middle;
      { left, middle, right };
    }
    else if (rightCount === 1) {
      left,
      middle,
      right: [||],
    }
    else if (leftCount > 0) {
      left: CopyOnWriteArray.removeLastOrRaise left,
      middle,
      right,
    }
    else failwith "vector is empty";
  };

let return (value: 'a): (t 'a) =>
  empty () |> addLast value;

let update
  (index: int)
    (value: 'a)
    ({ left, middle, right } as vector: t 'a): (t 'a) => {
  Preconditions.failIfOutOfRange (count vector) index;

  let leftCount = CopyOnWriteArray.count left;
  let middleCount = IndexedTrie.count middle;

  let rightIndex = index - middleCount - leftCount;

  if (index < leftCount) {
    left: left |>  CopyOnWriteArray.update index value,
    middle,
    right,
  }
  else if (rightIndex >= 0) {
    left,
    middle,
    right: right |> CopyOnWriteArray.update rightIndex value,
  }
  else {
    let index = (index - leftCount);
    let middle = middle |> IndexedTrie.updateUsingMutator
      IndexedTrie.updateLevelPersistent
      IndexedTrie.updateLeafPersistent
      Transient.Owner.none
      index
      value;

    { left, middle, right }
  };
};

let updateWith
    (index: int)
    (f: 'a => 'a)
    ({ left, middle, right } as vector: t 'a): (t 'a) => {
  Preconditions.failIfOutOfRange (count vector) index;

  let leftCount = CopyOnWriteArray.count left;
  let middleCount = IndexedTrie.count middle;

  let rightIndex = index - middleCount - leftCount;

  if (index < leftCount) {
    left: left |>  CopyOnWriteArray.updateWith index f,
    middle,
    right,
  }
  else if (rightIndex >= 0) {
    left,
    middle,
    right: right |> CopyOnWriteArray.updateWith rightIndex f,
  }
  else {
    let index = (index - leftCount);
    let middle = middle |> IndexedTrie.updateWithUsingMutator
      IndexedTrie.updateLevelPersistent
      IndexedTrie.updateLeafPersistent
      Transient.Owner.none
      index
      f;

    { left, middle, right }
  };
};
