/**
 * Copyright (c) 2017 - present Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

open Functions.Operators;

let module Ops = {
  type t 'a 'collection = {
    count: 'collection => int,
    first: 'collection => (option 'a),
    firstOrRaise: 'collection => 'a,
    toCollection: 'collection => Collection.t 'a,
    toIterable: 'collection => Iterable.t 'a,
    toSequence: 'collection => Sequence.t 'a,
  };
};

type t 'a =
  | Empty
  | SequentialCollection 'collection (Ops.t 'a 'collection): t 'a;

let count (collection: t 'a): int => switch collection {
  | Empty => 0
  | SequentialCollection collection { count } => count collection
};

let empty (): (t 'a) => Empty;

let first (collection: t 'a): (option 'a) => switch collection {
  | Empty => None
  | SequentialCollection collection { first } => first collection
};

let firstOrRaise (collection: t 'a): 'a => switch collection {
  | Empty => failwith "empty"
  | SequentialCollection collection { firstOrRaise } => firstOrRaise collection
};

let isEmpty (collection: t 'a): bool =>
  (count collection) === 0;

let isNotEmpty (collection: t 'a): bool =>
  (count collection) !== 0;

let toCollection (collection: t 'a): (Collection.t 'a) => switch collection {
  | Empty => Collection.empty ()
  | SequentialCollection collection { toCollection } => toCollection collection
};

let toIterable (collection: t 'a): (Iterable.t 'a) => switch collection {
  | Empty => Iterable.empty ()
  | SequentialCollection collection { toIterable } => toIterable collection
};

let toSequence (collection: t 'a): (Sequence.t 'a) => switch collection {
  | Empty => Sequence.empty ()
  | SequentialCollection collection { toSequence } => toSequence collection
};

let reduce
    while_::(predicate: 'acc => 'a => bool)=Functions.alwaysTrue2
    (f: 'acc => 'a => 'acc)
    (acc: 'acc)
    (collection: t 'a): 'acc =>
  collection |> toIterable |> Iterable.reduce while_::predicate f acc;

let toSequentialCollection (collection: t 'a): (t 'a) =>
  collection;

let map (f: 'a => 'b) (collection: t 'a): (t 'b) => switch collection {
  | Empty => Empty
  | SequentialCollection collection ops => SequentialCollection collection {
      count: ops.count,
      first: ops.first >> Option.map f,
      firstOrRaise: ops.firstOrRaise >> f,
      toCollection: ops.toCollection >> Collection.map f,
      toIterable: ops.toIterable >> Iterable.map f,
      toSequence: ops.toSequence >> Sequence.map f,
    }
};

let module Reducer = Iterable.Reducer.Make1 {
  type nonrec t 'a = t 'a;
  let reduce = reduce;
  let toIterable = toIterable;
};